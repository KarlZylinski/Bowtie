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
		case RenderResourceData::Target:
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

RenderResourceHandle Renderer::create_drawable(Allocator& allocator, const DrawableResourceData& drawable_data)
{
	RenderDrawable& drawable = *(RenderDrawable*)allocator.allocate(sizeof(RenderDrawable));
	drawable.texture = drawable_data.texture;
	drawable.model = drawable_data.model;
	drawable.shader = drawable_data.shader;
	drawable.geometry = drawable_data.geometry;
	drawable.num_vertices = drawable_data.num_vertices;
	return RenderResourceHandle(&drawable);
}

RenderResourceHandle Renderer::create_resource(Allocator& allocator, IConcreteRenderer& concrete_renderer, void* dynamic_data,
	const RenderResourceData& render_resource, Array<RenderTarget*>& render_targets, const RenderResourceLookupTable& resource_lut)
{
	switch(render_resource.type)
	{
	case RenderResourceData::Shader:
		return concrete_renderer.load_shader(*(ShaderResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Texture:
		return concrete_renderer.load_texture(*(TextureResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Drawable:
		{
			const auto& data = *(DrawableResourceData*)render_resource.data;
			auto handle = create_drawable(allocator, data);			
			auto& rw = *(RenderWorld*)resource_lut.lookup(data.render_world).render_object;
			rw.add_drawable(handle);
			return handle;
		}
		break;
	case RenderResourceData::World:
		return create_world(allocator, concrete_renderer); break;
	case RenderResourceData::Geometry:
		return concrete_renderer.load_geometry(*(GeometryResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Target:
		{
			auto render_target = concrete_renderer.create_render_target();
			array::push_back(render_targets, render_target);
			return RenderResourceHandle(render_target);
		}
		break;
	default:
		assert(!"Unknown render resource type"); return RenderResourceHandle();
	}
}

RenderResourceHandle Renderer::create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer)
{
	return RenderResourceHandle(MAKE_NEW(allocator, RenderWorld, allocator, *concrete_renderer.create_render_target()));
}

void Renderer::consume_command_queue()
{
	move_unprocessed_commands(_command_queue, _unprocessed_commands, _unprocessed_commands_mutex);
	notify_unprocessed_commands_consumed();

	for (unsigned i = 0; i < array::size(_command_queue); ++i)
		execute_command(_command_queue[i]);
	
	move_processed_commads(_command_queue, _processed_memory, _processed_memory_mutex);
	array::clear(_command_queue);
}

void Renderer::consume_create_resource(void* dynamic_data, const RenderResourceData& render_resource)
{
	auto handle = create_resource(_allocator, _concrete_renderer, dynamic_data, render_resource, _render_targets, _resource_lut);
	assert(handle.type != RenderResourceHandle::NotInitialized && "Failed to load resource!");
						
	// Map handle from outside of renderer (ResourceHandle) to internal handle (RenderResourceHandle).
	_resource_lut.set(render_resource.handle, handle);

	// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
	if (handle.type == RenderResourceHandle::Object)
		array::push_back(_resource_objects, RendererResourceObject(render_resource.type, handle));
	
	std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
	array::push_back(_processed_memory, render_resource.data);
}

void Renderer::drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data)
{
	drawable.model = data.model;
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
			consume_create_resource(command.dynamic_data, data);
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

void Renderer::flip(RendererContext& context)
{
	context.flip();
}

void Renderer::move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex)
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

void Renderer::move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex)
{
	std::lock_guard<std::mutex> queue_lock(unprocessed_commands_mutex);

	for(unsigned i = 0; i < array::size(unprocessed_commands); ++i)
		array::push_back(command_queue, unprocessed_commands[i]);		
		
	array::clear(unprocessed_commands);
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

void Renderer::raise_fence(RenderFence& fence)
{
	std::lock_guard<std::mutex> fence_lock(fence.mutex);
	fence.processed = true;
	fence.fence_processed.notify_all();
}

void Renderer::render_world(IConcreteRenderer& concrete_renderer, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view)
{
	concrete_renderer.set_render_target(render_world.render_target());
	concrete_renderer.clear();
	concrete_renderer.draw(view, render_world);
	array::push_back(rendered_worlds, &render_world);
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

}