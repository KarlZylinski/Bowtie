#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <engine/render_fence.h>
#include <engine/shader_utils.h>
#include <foundation/array.h>
#include <foundation/file.h>
#include <foundation/murmur_hash.h>
#include <foundation/temp_allocator.h>
#include <foundation/string_utils.h>
#include "concrete_renderer.h"
#include "render_material.h"
#include "render_drawable.h"
#include "render_world.h"
#include "render_target.h"
#include "render_texture.h"

namespace bowtie
{

////////////////////////////////
// Implementation fordward decl.

namespace
{

RenderResource create_drawable(Allocator& allocator, const RenderResourceLookupTable& resource_lut, const DrawableResourceData& data);
RenderResource create_geometry(ConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data);
RenderResource create_material(Allocator& allocator, ConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResourceLookupTable& lookup_table, const MaterialResourceData& data);
RenderResource create_render_target(ConcreteRenderer& concrete_renderer, Allocator& allocator, RenderResource texture, Array<RenderTarget*>& render_targets);
RenderResource create_shader(ConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data);
RenderResource create_texture(ConcreteRenderer& concrete_renderer, Allocator& allocator, image::PixelFormat pixel_format, const Vector2u& resolution, void* data);
RenderResource create_world(Allocator& allocator, RenderResource render_target);
void drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data);
void flip(IRendererContext& context);
void move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex);
void move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex);
void raise_fence(RenderFence& fence);
void render_world(ConcreteRenderer& concrete_renderer, const Vector2u& resolution, const RenderResourceLookupTable& resource_lut, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view);
RenderResource update_shader(ConcreteRenderer& concrete_renderer, const RenderResource& shader, void* dynamic_data, const ShaderResourceData& data);

}

////////////////////////////////
// Public interface.

Renderer::Renderer(ConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, RenderResourceLookupTable& render_resource_lookup_table) :
	_allocator(renderer_allocator), _concrete_renderer(concrete_renderer), _resource_lut(render_resource_lookup_table), _command_queue(array::create<RendererCommand>(_allocator)), _free_handles(array::create<RenderResourceHandle>(_allocator)),
	_unprocessed_commands(array::create<RendererCommand>(_allocator)), _processed_memory(array::create<void*>(_allocator)), _render_interface(*this, render_interface_allocator), _context(nullptr), _setup(false), _shut_down(false),
	_resource_objects(array::create<RendererResourceObject>(_allocator)), _render_targets(array::create<RenderTarget*>(_allocator)), _rendered_worlds(array::create<RenderWorld*>(_allocator))
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
		auto object = _resource_lut.lookup(resource_object.handle).object;
		switch (resource_object.type)
		{
		case RenderResourceData::World:
			_allocator.destroy((RenderWorld*)object);
			break;
		case RenderResourceData::RenderTarget: {
				auto rt = (RenderTarget*)object;
				_allocator.deallocate(rt->texture.object);
				_allocator.deallocate(rt);
			} break;
		case RenderResourceData::RenderMaterial: {
			auto rm = (RenderMaterial*)object;
			render_material::deinit(*rm, _allocator);
			_allocator.deallocate(rm);
		} break;
			
			
			
		default:
			_allocator.deallocate(object);
			break;
		}
	}
	
	array::deinit(_free_handles);
	array::deinit(_command_queue);
	array::deinit(_unprocessed_commands);
	array::deinit(_processed_memory);
	array::deinit(_resource_objects);
	array::deinit(_render_targets);
	array::deinit(_rendered_worlds);
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
	return _resoultion;
}

void Renderer::run(IRendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_resoultion = resolution;
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

RenderResource Renderer::create_resource(const RenderResourceData& data, void* dynamic_data)
{
	switch(data.type)
	{
		case RenderResourceData::Drawable: return create_drawable(_allocator, _resource_lut, *(DrawableResourceData*)data.data);
		case RenderResourceData::Geometry: return create_geometry(_concrete_renderer, dynamic_data, *(GeometryResourceData*)data.data);
		case RenderResourceData::RenderMaterial: return create_material(_allocator, _concrete_renderer, dynamic_data, _resource_lut, *(MaterialResourceData*)data.data);
		case RenderResourceData::RenderTarget: return create_render_target(_concrete_renderer, _allocator, create_texture(_concrete_renderer, _allocator, image::RGBA, _resoultion, 0), _render_targets);
		case RenderResourceData::Shader: return create_shader(_concrete_renderer, dynamic_data, *(ShaderResourceData*)data.data);
		case RenderResourceData::Texture: {
			auto texture_resource_data = (TextureResourceData*)data.data;
			auto texture_bits = memory::pointer_add(dynamic_data, texture_resource_data->texture_data_dynamic_data_offset);
			return create_texture(_concrete_renderer, _allocator, texture_resource_data->pixel_format, texture_resource_data->resolution, texture_bits);
		}
		case RenderResourceData::World: {
			return create_world(_allocator, create_render_target(_concrete_renderer, _allocator, create_texture(_concrete_renderer, _allocator, image::RGBA, _resoultion, 0), _render_targets));
		}
		default: assert(!"Unknown render resource type"); return RenderResource();
	}
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
			render_world(_concrete_renderer, _resoultion, _resource_lut, _rendered_worlds, *(RenderWorld*)_resource_lut.lookup(rwd.render_world).object, rwd.view);
		}
		break;

	case RendererCommand::LoadResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;

			auto handle = create_resource(data, dynamic_data);
			assert(handle.type != RenderResource::NotInitialized && "Failed to load resource!");

			// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
			_resource_lut.set(data.handle, handle);

			// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
			if (handle.type == RenderResource::Object)
				array::push_back(_resource_objects, RendererResourceObject(data.type, data.handle));
	
			std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
			array::push_back(_processed_memory, data.data);
		}
		break;

	case RendererCommand::UpdateResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			const auto& resource = _resource_lut.lookup(data.handle);
			
			if (resource.type == RenderResource::Object)
				array::remove(_resource_objects, [&](const RendererResourceObject& rro) { return data.handle == rro.handle; });

			auto handle = update_resource(data, dynamic_data, resource);
			assert(handle.type != RenderResource::NotInitialized && "Failed to load resource!");

			// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
			_resource_lut.set(data.handle, handle);

			// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
			if (handle.type == RenderResource::Object)
				array::push_back(_resource_objects, RendererResourceObject(data.type, data.handle));
	
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
			drawable_state_reflection(*(RenderDrawable*)_resource_lut.lookup(data.drawble).object, data);
		}
		break;

	case RendererCommand::DrawableGeometryReflection:
		{
			auto drawable_geometry_update_data = (DrawableGeometryReflectionData*)command.data;
			auto drawable = (RenderDrawable*)_resource_lut.lookup(drawable_geometry_update_data->drawable).object;
			_concrete_renderer.update_geometry(*drawable, command.dynamic_data, drawable_geometry_update_data->size);
		}
		break;

	case RendererCommand::CombineRenderedWorlds:
		{
			_concrete_renderer.unset_render_target(_resoultion);
			_concrete_renderer.clear();
			_concrete_renderer.combine_rendered_worlds(_fullscreen_rendering_quad, _rendered_worlds_combining_shader, _rendered_worlds);
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
			_concrete_renderer.destroy_geometry(_resource_lut.lookup(render_drawable.geometry));
			array::remove(_resource_objects, [&](const RendererResourceObject& rro) { return unspawn_data.drawable == rro.handle; });
			_resource_lut.free(unspawn_data.drawable);
			_allocator.deallocate(&render_drawable);
		}
		break;

	case RendererCommand::SetUniformValue:
		{
			const auto& set_uniform_value_data = *(SetUniformValueData*)command.data;
			auto& material = *(RenderMaterial*)_resource_lut.lookup(set_uniform_value_data.material).object;
			switch (set_uniform_value_data.type)
			{
			case uniform::Float:
				render_material::set_uniform_float_value(material, _allocator, set_uniform_value_data.uniform_name, *(float*)command.dynamic_data);
				break;
			default:
				assert(!"Unknown uniform type");
				break;
			}
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

	{
		static const float fullscreen_quad_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
		};

		_fullscreen_rendering_quad = _concrete_renderer.create_geometry((void*)fullscreen_quad_data, sizeof(fullscreen_quad_data));
	}

	{
		auto shader_source = file::load("rendered_world_combining.shader", _allocator);
		auto split_shader = shader_utils::split_shader(shader_source, _allocator);
		_rendered_worlds_combining_shader = _concrete_renderer.create_shader(split_shader.vertex_source, split_shader.fragment_source);
		_allocator.deallocate(shader_source.data);
		_allocator.deallocate(split_shader.vertex_source);
		_allocator.deallocate(split_shader.fragment_source);
	}

	while (_active)
	{
		wait_for_unprocessed_commands_to_exist();
		consume_command_queue();
	}
}

RenderResource Renderer::update_resource(const RenderResourceData& data, void* dynamic_data, const RenderResource& resource)
{
	switch(data.type)
	{
		case RenderResourceData::Shader: return update_shader(_concrete_renderer, resource, dynamic_data, *(ShaderResourceData*)data.data);
		default: assert(!"Unknown render resource type"); return RenderResource();
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

	drawable.texture = data.texture;
	drawable.model = data.model;
	drawable.material = data.material;
	drawable.geometry = data.geometry;
	drawable.num_vertices = data.num_vertices;
	drawable.depth = data.depth;
	auto& rw = *(RenderWorld*)resource_lut.lookup(data.render_world).object;
	rw.add_drawable(&drawable);
	return RenderResource(&drawable);
}

RenderResource create_geometry(ConcreteRenderer& concrete_renderer, void* dynamic_data, const GeometryResourceData& data)
{
	return concrete_renderer.create_geometry(dynamic_data, data.size);
}

RenderUniform create_uniform(ConcreteRenderer& concrete_renderer, RenderResource shader, const UniformResourceData& uniform_data)
{
	auto location = concrete_renderer.get_uniform_location(shader, uniform_data.name);
	auto name_hash = hash_str(uniform_data.name);

	if (uniform_data.automatic_value == uniform::None)
		return RenderUniform(uniform_data.type, name_hash, location);
	else
		return RenderUniform(uniform_data.type, name_hash, location, uniform_data.automatic_value);
}

RenderResource create_material(Allocator& allocator, ConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResourceLookupTable& lookup_table, const MaterialResourceData& data)
{
	auto material = (RenderMaterial*)allocator.allocate(sizeof(RenderMaterial));
	render_material::init(*material, allocator, data.shader);
	auto shader = lookup_table.lookup(data.shader);
	auto uniforms_data = (UniformResourceData*)dynamic_data;
	
	for (unsigned i = 0; i < data.num_uniforms; ++i)
	{
		const auto& uniform_data = uniforms_data[i];
		auto uniform = create_uniform(concrete_renderer, shader, uniform_data);

		if (uniform_data.value != nullptr)
		{
			switch (uniform_data.type)
			{
			case uniform::Float:
				render_uniform::set_value(uniform, allocator, uniform_data.value, sizeof(float));
				break;
			case uniform::Texture1:
			case uniform::Texture2:
			case uniform::Texture3:
				render_uniform::set_value(uniform, allocator, uniform_data.value, sizeof(unsigned));
				break;
			case uniform::Vec4:
				render_uniform::set_value(uniform, allocator, uniform_data.value, sizeof(Vector4));
				break;
			default:
				assert(!"Unkonwn uniform type.");
				break;
			}
		}

		render_material::add_uniform(*material, uniform);
	}

	return RenderResource(material);
}

RenderResource create_render_target(ConcreteRenderer& concrete_renderer, Allocator& allocator, RenderResource texture, Array<RenderTarget*>& render_targets)
{
	auto render_target_resource = concrete_renderer.create_render_target((RenderTexture*)texture.object);
	auto render_target = (RenderTarget*)allocator.allocate(sizeof(RenderTarget));
	render_target->handle = render_target_resource;
	render_target->texture = texture;
	array::push_back(render_targets, render_target);
	return RenderResource(render_target);
}

RenderResource create_shader(ConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data)
{
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	return concrete_renderer.create_shader(vertex_source, fragment_source);
}

RenderResource create_texture(ConcreteRenderer& concrete_renderer, Allocator& allocator, image::PixelFormat pixel_format, const Vector2u& resolution, void* data)
{
	auto texture_resource = concrete_renderer.create_texture(pixel_format, resolution, data);
	RenderTexture* render_texture = (RenderTexture*)allocator.allocate(sizeof(RenderTexture));
	render_texture->pixel_format = pixel_format;
	render_texture->render_handle = texture_resource;
	render_texture->resolution = resolution;
	return RenderResource(render_texture);
}

RenderResource create_world(Allocator& allocator, RenderResource render_target)
{
	return RenderResource(allocator.construct<RenderWorld>(allocator, *(RenderTarget*)render_target.object));
}

void drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data)
{
	drawable.model = data.model;
	drawable.depth = data.depth;
	drawable.material = data.material;
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

void render_world(ConcreteRenderer& concrete_renderer, const Vector2u& resolution, const RenderResourceLookupTable& resource_lut, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view)
{
	render_world.sort();
	concrete_renderer.set_render_target(resolution, render_world.render_target().handle);
	concrete_renderer.clear();
	concrete_renderer.draw(view, render_world, resolution, resource_lut);
	array::push_back(rendered_worlds, &render_world);
}

RenderResource update_shader(ConcreteRenderer& concrete_renderer, const RenderResource& shader, void* dynamic_data, const ShaderResourceData& data)
{
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	return concrete_renderer.update_shader(shader, vertex_source, fragment_source);
}

} // implementation

} // namespace bowtie
