#include "renderer.h"

#include <cassert>

#include <engine/render_fence.h>
#include <foundation/array.h>
#include <foundation/queue.h>

#include "render_drawable.h"
#include "render_world.h"
#include "render_target.h"
#include "iconcrete_renderer.h"

namespace bowtie
{

////////////////////////////////
// Implementation fordward decl.

namespace
{


#define CreateResourceFunc(arguments) std::function<RenderResourceHandle(arguments)>

struct ResourceCreators
{
	ResourceCreators(const CreateResourceFunc(const DrawableResourceData&)& create_drawable,
		const CreateResourceFunc(const GeometryResourceData&)& create_geometry,
		const CreateResourceFunc(void)& create_render_target,
		const CreateResourceFunc(const ShaderResourceData&)& create_shader,
		const CreateResourceFunc(const TextureResourceData&)& create_texture,
		const CreateResourceFunc(void)& create_world)
			: create_drawable(create_drawable), create_geometry(create_geometry), create_render_target(create_render_target),
			  create_shader(create_shader), create_texture(create_texture), create_world(create_world) {}

	CreateResourceFunc(const DrawableResourceData&) create_drawable;
	CreateResourceFunc(const GeometryResourceData&) create_geometry;
	CreateResourceFunc(void) create_render_target;
	CreateResourceFunc(const ShaderResourceData&) create_shader;
	CreateResourceFunc(const TextureResourceData&) create_texture;
	CreateResourceFunc(void) create_world;
};

RenderResourceHandle create_drawable(Allocator& allocator, const RenderResourceLookupTable& resource_lut, const DrawableResourceData& data);
RenderResourceHandle create_geometry(IConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data);
RenderResourceHandle create_render_target(IConcreteRenderer& concrete_renderer, Array<RenderTarget*>& render_targets);
RenderResourceHandle create_resource(const RenderResourceData& data, ResourceCreators& resource_creators);
RenderResourceHandle create_shader(IConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data);
RenderResourceHandle create_texture(IConcreteRenderer& concrete_renderer, void* dynamic_data, const TextureResourceData& data);
RenderResourceHandle create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer);
void drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data);
void flip(RendererContext& context);
void move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex);
void move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex);
void raise_fence(RenderFence& fence);
void render_world(IConcreteRenderer& concrete_renderer, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view);

}

////////////////////////////////
// Public interface.

Renderer::Renderer(IConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, RenderResourceLookupTable& render_resource_lookup_table) :
	_allocator(renderer_allocator), _concrete_renderer(concrete_renderer), _resource_lut(render_resource_lookup_table), _command_queue(_allocator), _free_handles(_allocator),  _unprocessed_commands(_allocator),
	_processed_memory(_allocator), _render_interface(*this, render_interface_allocator), _context(nullptr), _setup(false), _resource_objects(_allocator),
	_render_targets(_allocator), _rendered_worlds(_allocator)
{
	auto num_handles = RenderResourceLookupTable::num_handles;
	array::set_capacity(_free_handles, num_handles);

	for(unsigned handle = num_handles; handle > 0; --handle)
		array::push_back(_free_handles, ResourceHandle(handle));
}

Renderer::~Renderer()
{
	_active = false;
	notify_unprocessed_commands_exists();
	_thread.join();

	for (unsigned i = 0; i < array::size(_resource_objects); ++i)
	{
		auto& resource_object = _resource_objects[i];
		switch (resource_object.type)
		{
		case RenderResourceData::World:
			MAKE_DELETE(_allocator, RenderWorld, (RenderWorld*)resource_object.handle.render_object);
			break;
		case RenderResourceData::RenderTarget:
			MAKE_DELETE(_allocator, RenderTarget, (RenderTarget*)resource_object.handle.render_object);
			break;
		default:
			_allocator.deallocate(resource_object.handle.render_object);
			break;
		}
	}
}

void Renderer::add_renderer_command(const RendererCommand& command)
{
	{
		std::lock_guard<std::mutex> queue_lock(_unprocessed_commands_mutex);
		array::push_back(_unprocessed_commands, command);
	}

	notify_unprocessed_commands_exists();
}

ResourceHandle Renderer::create_handle()
{
	assert(array::any(_free_handles) && "Out of render resource handles!");

	ResourceHandle handle = array::back(_free_handles);
	array::pop_back(_free_handles);

	return handle;
}

void Renderer::deallocate_processed_commands(Allocator& allocator)
{
	std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);

	for (unsigned i = 0; i < array::size(_processed_memory); ++i)
	{
		void* ptr = _processed_memory[i];
		allocator.deallocate(ptr);
	}

	array::clear(_processed_memory);
}

bool Renderer::is_active() const
{
	return _active;
}

bool Renderer::is_setup() const
{
	return _setup;
}

RenderInterface& Renderer::render_interface()
{
	return _render_interface;
}

const Vector2u& Renderer::resolution() const
{
	return _concrete_renderer.resolution();
}

void Renderer::run(RendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_thread = std::thread(&Renderer::thread, this);

	// Do stuff here which should happen before anything else.
	_render_interface.resize(resolution);
	_setup = true;
}


////////////////////////////////
// Implementation.

void Renderer::consume_command_queue()
{
	move_unprocessed_commands(_command_queue, _unprocessed_commands, _unprocessed_commands_mutex);
	notify_unprocessed_commands_consumed();

	for (unsigned i = 0; i < array::size(_command_queue); ++i)
		execute_command(_command_queue[i]);
	
	move_processed_commads(_command_queue, _processed_memory, _processed_memory_mutex);
	array::clear(_command_queue);
}

void Renderer::execute_command(const RendererCommand& command)
{
	switch(command.type)
	{
	case RendererCommand::Fence:
		raise_fence(*(RenderFence*)command.data);
		break;
			
	case RendererCommand::RenderWorld:
		{
			RenderWorldData& rwd = *(RenderWorldData*)command.data;
			render_world(_concrete_renderer, _rendered_worlds, *(RenderWorld*)_resource_lut.lookup(rwd.render_world).render_object, rwd.view);
		}
		break;

	case RendererCommand::LoadResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			
			ResourceCreators resource_creators(
				std::bind(&create_drawable, std::ref(_allocator), std::cref(_resource_lut), std::placeholders::_1),
				std::bind(&create_geometry, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_render_target, std::ref(_concrete_renderer), std::ref(_render_targets)),
				std::bind(&create_shader, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_texture, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_world, std::ref(_allocator), std::ref(_concrete_renderer))
			);

			auto handle = create_resource(data, resource_creators);
			assert(handle.type != RenderResourceHandle::NotInitialized && "Failed to load resource!");

			// Map handle from outside of renderer (ResourceHandle) to internal handle (RenderResourceHandle).
			_resource_lut.set(data.handle, handle);

			// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
			if (handle.type == RenderResourceHandle::Object)
				array::push_back(_resource_objects, RendererResourceObject(data.type, handle));
	
			std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
			array::push_back(_processed_memory, data.data);
		}
		break;

	case RendererCommand::Resize:
		{
			ResizeData& data = *(ResizeData*)command.data;
			_concrete_renderer.resize(data.resolution, _render_targets);
		}
		break;
	case RendererCommand::DrawableStateReflection:
		{
			const auto& data = *(DrawableStateReflectionData*)command.data;
			drawable_state_reflection(*(RenderDrawable*)_resource_lut.lookup(data.drawble).render_object, data);
		}
		break;
	case RendererCommand::DrawableGeometryReflection:
		{
			_concrete_renderer.update_geometry(*(DrawableGeometryReflectionData*)command.data, command.dynamic_data);
		}
		break;
	case RendererCommand::CombineRenderedWorlds:
		{
			_concrete_renderer.combine_rendered_worlds(_rendered_worlds);
			array::clear(_rendered_worlds);
			flip(*_context);
		}
		break;
	default:
		assert(!"Command not implemented!");
		break;
	}
}

void Renderer::notify_unprocessed_commands_consumed()
{
	std::unique_lock<std::mutex> unprocessed_commands_exists_lock(_unprocessed_commands_exists_mutex);
	_unprocessed_commands_exists = false;
}

void Renderer::notify_unprocessed_commands_exists()
{
	std::lock_guard<std::mutex>	unprocessed_commands_exists_lock(_unprocessed_commands_exists_mutex);
	_unprocessed_commands_exists = true;
	_wait_for_unprocessed_commands_to_exist.notify_all();
}

void Renderer::thread()
{
	_context->make_current_for_calling_thread();
	_concrete_renderer.initialize_thread();
	_active = true;

	while (_active)
	{
		wait_for_unprocessed_commands_to_exist();
		consume_command_queue();
	}
}

void Renderer::wait_for_unprocessed_commands_to_exist()
{	
	std::unique_lock<std::mutex> unprocessed_commands_exists_lock(_unprocessed_commands_exists_mutex);
	_wait_for_unprocessed_commands_to_exist.wait(unprocessed_commands_exists_lock, [&]{return _unprocessed_commands_exists;});
}

namespace
{

RenderResourceHandle create_drawable(Allocator& allocator, const RenderResourceLookupTable& resource_lut, const DrawableResourceData& data)
{
	RenderDrawable& drawable = *(RenderDrawable*)allocator.allocate(sizeof(RenderDrawable));
	drawable.texture = data.texture;
	drawable.model = data.model;
	drawable.shader = data.shader;
	drawable.geometry = data.geometry;
	drawable.num_vertices = data.num_vertices;
	auto handle = RenderResourceHandle(&drawable);
	auto& rw = *(RenderWorld*)resource_lut.lookup(data.render_world).render_object;
	rw.add_drawable(handle);
	return handle;
}

RenderResourceHandle create_geometry(IConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data)
{
	return concrete_renderer.load_geometry(data, dynamic_data);
}

RenderResourceHandle create_render_target(IConcreteRenderer& concrete_renderer, Array<RenderTarget*>& render_targets)
{
	auto render_target = concrete_renderer.create_render_target();
	array::push_back(render_targets, render_target);
	return RenderResourceHandle(render_target);
}

RenderResourceHandle create_resource(const RenderResourceData& data, ResourceCreators& resource_creators)
{
	switch(data.type)
	{
		case RenderResourceData::Drawable: return resource_creators.create_drawable(*(DrawableResourceData*)data.data);
		case RenderResourceData::Geometry: return resource_creators.create_geometry(*(GeometryResourceData*)data.data);
		case RenderResourceData::RenderTarget: return resource_creators.create_render_target();
		case RenderResourceData::Shader: return resource_creators.create_shader(*(ShaderResourceData*)data.data);
		case RenderResourceData::Texture: return resource_creators.create_texture(*(TextureResourceData*)data.data);
		case RenderResourceData::World: return resource_creators.create_world();
		default: assert(!"Unknown render resource type"); return RenderResourceHandle();
	}
}

RenderResourceHandle create_shader(IConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data)
{
	return concrete_renderer.load_shader(data, dynamic_data);
}

RenderResourceHandle create_texture(IConcreteRenderer& concrete_renderer, void* dynamic_data, const TextureResourceData& data)
{
	return concrete_renderer.load_texture(data, dynamic_data);
}

RenderResourceHandle create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer)
{
	return RenderResourceHandle(MAKE_NEW(allocator, RenderWorld, allocator, *concrete_renderer.create_render_target()));
}

void drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data)
{
	drawable.model = data.model;
}

void flip(RendererContext& context)
{
	context.flip();
}

void move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex)
{
	std::lock_guard<std::mutex> queue_lock(processed_memory_mutex);

	for (unsigned i = 0; i < array::size(command_queue); ++i) {
		const auto& command = command_queue[i];
		auto dont_free = command.type == RendererCommand::Fence;

		if (dont_free)
			continue;

		array::push_back(processed_memory, command.data);
		array::push_back(processed_memory, command.dynamic_data);
	}
}

void move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex)
{
	std::lock_guard<std::mutex> queue_lock(unprocessed_commands_mutex);

	for(unsigned i = 0; i < array::size(unprocessed_commands); ++i)
		array::push_back(command_queue, unprocessed_commands[i]);		
		
	array::clear(unprocessed_commands);
}

void raise_fence(RenderFence& fence)
{
	std::lock_guard<std::mutex> fence_lock(fence.mutex);
	fence.processed = true;
	fence.fence_processed.notify_all();
}

void render_world(IConcreteRenderer& concrete_renderer, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view)
{
	concrete_renderer.set_render_target(render_world.render_target());
	concrete_renderer.clear();
	concrete_renderer.draw(view, render_world);
	array::push_back(rendered_worlds, &render_world);
}

} // implementation

} // namespace bowtie
