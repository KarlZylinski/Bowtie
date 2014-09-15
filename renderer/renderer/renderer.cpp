#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <engine/render_fence.h>
#include <foundation/array.h>
#include <foundation/murmur_hash.h>
#include <foundation/temp_allocator.h>
#include <foundation/string_utils.h>
#include <foundation/queue.h>
#include "iconcrete_renderer.h"
#include "render_material.h"
#include "render_drawable.h"
#include "render_world.h"
#include "render_target.h"

namespace bowtie
{

////////////////////////////////
// Implementation fordward decl.

namespace
{


#define CreateResourceFunc(arguments) std::function<RenderResource(arguments)>

struct ResourceCreators
{
	ResourceCreators(const CreateResourceFunc(const DrawableResourceData&)& create_drawable,
		const CreateResourceFunc(const GeometryResourceData&)& create_geometry,
		const CreateResourceFunc(const MaterialResourceData&)& create_material,
		const CreateResourceFunc(void)& create_render_target,
		const CreateResourceFunc(const ShaderResourceData&)& create_shader,
		const CreateResourceFunc(const TextureResourceData&)& create_texture,
		const CreateResourceFunc(void)& create_world)
		: create_drawable(create_drawable), create_geometry(create_geometry), create_material(create_material), create_render_target(create_render_target),
			  create_shader(create_shader), create_texture(create_texture), create_world(create_world) {}

	CreateResourceFunc(const DrawableResourceData&) create_drawable;
	CreateResourceFunc(const GeometryResourceData&) create_geometry;
	CreateResourceFunc(const MaterialResourceData&) create_material;
	CreateResourceFunc(void) create_render_target;
	CreateResourceFunc(const ShaderResourceData&) create_shader;
	CreateResourceFunc(const TextureResourceData&) create_texture;
	CreateResourceFunc(void) create_world;
};


#define UpdateResourceFunc(arguments) std::function<RenderResource(arguments)>

struct ResourceUpdaters
{
	ResourceUpdaters(const UpdateResourceFunc(const ShaderResourceData&)& update_shader)
		: update_shader(update_shader) {}

	UpdateResourceFunc(const ShaderResourceData&) update_shader;
};

RenderResource create_drawable(Allocator& allocator, const RenderResourceLookupTable& resource_lut, const DrawableResourceData& data);
RenderResource create_geometry(IConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data);
RenderResource create_material(Allocator& allocator, IConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResourceLookupTable& lookup_table, const MaterialResourceData& data);
RenderResource create_render_target(IConcreteRenderer& concrete_renderer, Array<RenderTarget*>& render_targets);
RenderResource create_resource(const RenderResourceData& data, ResourceCreators& resource_creators);
RenderResource create_shader(IConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data);
RenderResource create_texture(IConcreteRenderer& concrete_renderer, void* dynamic_data, const TextureResourceData& data);
RenderResource create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer);
void drawable_state_reflection(RenderDrawable& drawable, const RenderResourceLookupTable& resource_lut, const DrawableStateReflectionData& data);
void flip(IRendererContext& context);
void move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex);
void move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex);
void raise_fence(RenderFence& fence);
void render_world(IConcreteRenderer& concrete_renderer, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view);
RenderResource update_resource(const RenderResourceData& data, ResourceUpdaters& resource_updaters);
RenderResource update_shader(IConcreteRenderer& concrete_renderer, const RenderResource& shader, void* dynamic_data, const ShaderResourceData& data);

}

////////////////////////////////
// Public interface.

Renderer::Renderer(IConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, RenderResourceLookupTable& render_resource_lookup_table) :
	_allocator(renderer_allocator), _concrete_renderer(concrete_renderer), _resource_lut(render_resource_lookup_table), _command_queue(_allocator), _free_handles(_allocator),  _unprocessed_commands(_allocator),
	_processed_memory(_allocator), _render_interface(*this, render_interface_allocator), _context(nullptr), _setup(false), _shut_down(false), _resource_objects(_allocator),
	_render_targets(_allocator), _rendered_worlds(_allocator)
{
	auto num_handles = RenderResourceLookupTable::num_handles;
	array::set_capacity(_free_handles, num_handles);
	
	for(unsigned handle = num_handles; handle > 0; --handle)
		array::push_back(_free_handles, RenderResourceHandle(handle));
}

Renderer::~Renderer()
{	
	for (unsigned i = 0; i < array::size(_resource_objects); ++i)
	{
		auto& resource_object = _resource_objects[i];
		switch (resource_object.type)
		{
		case RenderResourceData::World:
			_allocator.destroy((RenderWorld*)resource_object.handle.object);
			break;
		case RenderResourceData::RenderTarget:
			_allocator.destroy((RenderTarget*)resource_object.handle.object);
			break;
		case RenderResourceData::RenderMaterial:
			_allocator.destroy((RenderMaterial*)resource_object.handle.object);
			break;
		default:
			_allocator.deallocate(resource_object.handle.object);
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

	if (_shut_down) { // Move to trash, right away!
		move_processed_commads(_unprocessed_commands, _processed_memory, _processed_memory_mutex);
		array::clear(_unprocessed_commands);
	} else 
		notify_unprocessed_commands_exists();
}

RenderResourceHandle Renderer::create_handle()
{
	assert(array::any(_free_handles) && "Out of render resource handles!");
	RenderResourceHandle handle = array::back(_free_handles);
	array::pop_back(_free_handles);
	return handle;
}

void Renderer::deallocate_processed_commands(Allocator& render_interface_allocator)
{
	std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);

	for (unsigned i = 0; i < array::size(_processed_memory); ++i)
	{
		void* ptr = _processed_memory[i];
		render_interface_allocator.deallocate(ptr);
	}

	array::clear(_processed_memory);
}

void Renderer::free_handle(RenderResourceHandle handle)
{
	assert(handle < RenderResourceLookupTable::num_handles && "Trying to free render resource handle with a higher value than RenderResourceLookupTable::num_handles.");
	array::push_back(_free_handles, handle);
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

void Renderer::run(IRendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_thread = std::thread(&Renderer::thread, this);

	// Do stuff here which should happen before anything else.
	_render_interface.resize(resolution);
	_setup = true;
}

void Renderer::stop(Allocator& render_interface_allocator)
{
	_active = false;
	_setup = false;
	_shut_down = true;
	notify_unprocessed_commands_exists();
	_thread.join();
	move_processed_commads(_unprocessed_commands, _processed_memory, _processed_memory_mutex);
	array::clear(_unprocessed_commands);
	move_processed_commads(_command_queue, _processed_memory, _processed_memory_mutex);
	array::clear(_command_queue);
	deallocate_processed_commands(render_interface_allocator);
}


////////////////////////////////
// Implementation.

void Renderer::consume_command_queue()
{
	{
		std::unique_lock<std::mutex> unprocessed_commands_exists_lock(_unprocessed_commands_exists_mutex);
		move_unprocessed_commands(_command_queue, _unprocessed_commands, _unprocessed_commands_mutex);
		_unprocessed_commands_exists = false;
	}

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
			render_world(_concrete_renderer, _rendered_worlds, *(RenderWorld*)_resource_lut.lookup(rwd.render_world).object, rwd.view);
		}
		break;

	case RendererCommand::LoadResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			
			ResourceCreators resource_creators(
				std::bind(&create_drawable, std::ref(_allocator), std::cref(_resource_lut), std::placeholders::_1),
				std::bind(&create_geometry, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_material, std::ref(_allocator), std::ref(_concrete_renderer), dynamic_data, std::cref(_resource_lut), std::placeholders::_1),
				std::bind(&create_render_target, std::ref(_concrete_renderer), std::ref(_render_targets)),
				std::bind(&create_shader, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_texture, std::ref(_concrete_renderer), dynamic_data, std::placeholders::_1),
				std::bind(&create_world, std::ref(_allocator), std::ref(_concrete_renderer))
			);

			auto handle = create_resource(data, resource_creators);
			assert(handle.type != RenderResource::NotInitialized && "Failed to load resource!");

			// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
			_resource_lut.set(data.handle, handle);

			// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
			if (handle.type == RenderResource::Object)
				array::push_back(_resource_objects, RendererResourceObject(data.type, handle));
	
			std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
			array::push_back(_processed_memory, data.data);
		}
		break;

	case RendererCommand::UpdateResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			const auto& resource = _resource_lut.lookup(data.handle);
			ResourceUpdaters resource_updaters(
				std::bind(&update_shader, std::ref(_concrete_renderer), resource, dynamic_data, std::placeholders::_1)
			);

			auto handle = update_resource(data, resource_updaters);
			assert(handle.type != RenderResource::NotInitialized && "Failed to load resource!");

			// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
			_resource_lut.set(data.handle, handle);

			// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
			// TODO: FIX, need to store rrh instead of this shait			
			//if (handle.type == RenderResource::Object)
			//	array::push_back(_resource_objects, RendererResourceObject(data.type, handle));
	
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
			drawable_state_reflection(*(RenderDrawable*)_resource_lut.lookup(data.drawble).object, _resource_lut, data);
		}
		break;

	case RendererCommand::DrawableGeometryReflection:
		{
			_concrete_renderer.update_geometry(*(DrawableGeometryReflectionData*)command.data, command.dynamic_data);
		}
		break;

	case RendererCommand::CombineRenderedWorlds:
		{
			_concrete_renderer.unset_render_target();
			_concrete_renderer.clear();
			_concrete_renderer.combine_rendered_worlds(_rendered_worlds);
			array::clear(_rendered_worlds);
			flip(*_context);
		}
		break;

	case RendererCommand::Unspawn:
		{
			const auto& unspawn_data = *(UnspawnData*)command.data;
			auto& render_world = *(RenderWorld*)_resource_lut.lookup(unspawn_data.world).object;
			auto& render_drawable = *(RenderDrawable*)_resource_lut.lookup(unspawn_data.drawable).object;
			render_world.remove_drawable(&render_drawable);
			_concrete_renderer.unload_geometry(render_drawable.geometry);
			array::remove(_resource_objects, [&](const RendererResourceObject& rro) { return &render_drawable == rro.handle.object; });
			_resource_lut.free(unspawn_data.drawable);
			_allocator.deallocate(&render_drawable);
		}
		break;

	case RendererCommand::SetUniformValue:
		{
			const auto& set_uniform_value_data = *(SetUniformValueData*)command.data;
			auto& material = *(RenderMaterial*)_resource_lut.lookup(set_uniform_value_data.material).object;
			material.set_uniform_value(set_uniform_value_data.uniform_name, set_uniform_value_data.value);
		}
		break;

	default:
		assert(!"Command not implemented!");
		break;
	}
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

RenderResource create_drawable(Allocator& allocator, const RenderResourceLookupTable& resource_lut, const DrawableResourceData& data)
{
	RenderDrawable& drawable = *(RenderDrawable*)allocator.allocate(sizeof(RenderDrawable));

	drawable.texture = data.texture != RenderResourceHandle::NotInitialized
		? (RenderTexture*)resource_lut.lookup(data.texture).object
		: nullptr;

	drawable.model = data.model;
	drawable.material = (RenderMaterial*)resource_lut.lookup(data.material).object;
	drawable.geometry = resource_lut.lookup(data.geometry);
	drawable.num_vertices = data.num_vertices;
	auto& rw = *(RenderWorld*)resource_lut.lookup(data.render_world).object;
	rw.add_drawable(&drawable);
	return RenderResource(&drawable);
}

RenderResource create_geometry(IConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data)
{
	return concrete_renderer.load_geometry(data, dynamic_data);
}

Array<char*> split(Allocator& allocator, const char* str, char delimiter)
{
	Array<char*> words(allocator);
	auto len_str = strlen(str);

	if (len_str == 0)
		return words;

	auto len = len_str + 1;
	unsigned current_word_len = 0;

	for (unsigned i = 0; i < len; ++i)
	{
		if (str[i] == delimiter || i == len - 1)
		{
			char* word = (char*)allocator.allocate(current_word_len);
			memcpy(word, str + i - current_word_len, current_word_len);
			word[current_word_len] = 0;
			array::push_back(words, word);
			current_word_len = 0;
		}
		else
			++current_word_len;
	}

	return words;
}

Uniform::Type get_uniform_type_from_str(const char* str)
{
	static const char* types_as_str[] = { "float", "vec2", "vec3", "vec4", "mat3", "mat4" };
	
	for (unsigned i = 0; i < Uniform::NumUniformTypes; ++i)
	{
		if (!strcmp(str, types_as_str[i]))
			return (Uniform::Type)i;
	}

	assert(!"Unknown uniform type");
	return Uniform::NumUniformTypes;
}

Uniform::AutomaticValue get_automatic_value_from_str(const char* str)
{
	static const char* types_as_str[] = { "none", "mvp", "time" };
	
	for (unsigned i = 0; i < Uniform::NumAutomaticValues; ++i)
	{
		if (!strcmp(str, types_as_str[i]))
			return (Uniform::AutomaticValue)i;
	}

	return Uniform::None;
}

Vector4 get_value_from_str(const char* str)
{
	return Vector4((float)strtod(str, nullptr), 0, 0, 0);
}

RenderResource create_material(Allocator& allocator, IConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResourceLookupTable& lookup_table, const MaterialResourceData& data)
{
	auto material = allocator.construct<RenderMaterial>(allocator, data.shader);
	auto shader = lookup_table.lookup(data.shader);
	char* current_uniform = (char*)dynamic_data;
	
	for (unsigned i = 0; i < data.num_uniforms; ++i)
	{
		TempAllocator4096 ta;
		auto split_uniform = split(ta, current_uniform, ' ');
		assert(array::size(split_uniform) >= 2 && "Uniform definition must contain at least type and name.");
		auto type = get_uniform_type_from_str(split_uniform[0]);
		auto name = split_uniform[1];
		auto name_hash = hash_str(name);
		auto location = concrete_renderer.get_uniform_location(shader, name);
		
		if (array::size(split_uniform) > 2)
		{
			auto value_str = split_uniform[2];
			auto automatic_value = get_automatic_value_from_str(value_str);

			if (automatic_value != Uniform::None)
				material->add_uniform(Uniform(type, name_hash, location, automatic_value));
			else
				material->add_uniform(Uniform(type, name_hash, location, get_value_from_str(value_str)));
		}
		else
			material->add_uniform(Uniform(type, name_hash, location, Vector4()));

		current_uniform += strlen(current_uniform) + 1;
	}

	return RenderResource(material);
}

RenderResource create_render_target(IConcreteRenderer& concrete_renderer, Array<RenderTarget*>& render_targets)
{
	auto render_target = concrete_renderer.create_render_target();
	array::push_back(render_targets, render_target);
	return RenderResource(render_target);
}

RenderResource create_resource(const RenderResourceData& data, ResourceCreators& resource_creators)
{
	switch(data.type)
	{
		case RenderResourceData::Drawable: return resource_creators.create_drawable(*(DrawableResourceData*)data.data);
		case RenderResourceData::Geometry: return resource_creators.create_geometry(*(GeometryResourceData*)data.data);
		case RenderResourceData::RenderMaterial: return resource_creators.create_material(*(MaterialResourceData*)data.data);
		case RenderResourceData::RenderTarget: return resource_creators.create_render_target();
		case RenderResourceData::Shader: return resource_creators.create_shader(*(ShaderResourceData*)data.data);
		case RenderResourceData::Texture: return resource_creators.create_texture(*(TextureResourceData*)data.data);
		case RenderResourceData::World: return resource_creators.create_world();
		default: assert(!"Unknown render resource type"); return RenderResource();
	}
}

RenderResource create_shader(IConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data)
{
	return concrete_renderer.load_shader(data, dynamic_data);
}

RenderResource create_texture(IConcreteRenderer& concrete_renderer, void* dynamic_data, const TextureResourceData& data)
{
	return RenderResource(concrete_renderer.load_texture(data, dynamic_data));
}

RenderResource create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer)
{
	return RenderResource(allocator.construct<RenderWorld>(allocator, *concrete_renderer.create_render_target()));
}

void drawable_state_reflection(RenderDrawable& drawable, const RenderResourceLookupTable& resource_lut, const DrawableStateReflectionData& data)
{
	drawable.model = data.model;

	if (data.material != RenderResourceHandle::NotInitialized)
		drawable.material = (RenderMaterial*)resource_lut.lookup(data.material).object;
}

void flip(IRendererContext& context)
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

RenderResource update_resource(const RenderResourceData& data, ResourceUpdaters& resource_updaters)
{
	switch(data.type)
	{
		case RenderResourceData::Shader: return resource_updaters.update_shader(*(ShaderResourceData*)data.data);
		default: assert(!"Unknown render resource type"); return RenderResource();
	}
}

RenderResource update_shader(IConcreteRenderer& concrete_renderer, const RenderResource& shader, void* dynamic_data, const ShaderResourceData& data)
{
	return concrete_renderer.update_shader(shader, data, dynamic_data);
}

} // implementation

} // namespace bowtie
