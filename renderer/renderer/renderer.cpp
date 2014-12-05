#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <engine/render_fence.h>
#include <engine/shader_utils.h>
#include <engine/entity/components/sprite_renderer_component.h>
#include <foundation/array.h>
#include <foundation/file.h>
#include <foundation/murmur_hash.h>
#include <foundation/temp_allocator.h>
#include <foundation/string_utils.h>
#include "concrete_renderer.h"
#include "render_material.h"
#include "render_world.h"
#include "render_target.h"
#include "render_texture.h"
#include "render_component.h"
#include <engine/material.h>

namespace bowtie
{

////////////////////////////////
// Implementation fordward decl.

namespace
{

struct SingleCreatedResource
{
	RenderResourceHandle handle;
	RenderResource resource;
};

struct SingleUpdatedResource
{
	RenderResourceHandle handle;
	RenderResource old_resource;
	RenderResource new_resource;
};

SingleCreatedResource create_material(Allocator& allocator, ConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResource* resource_table, const MaterialResourceData& data);
RenderResource create_render_target_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, const RenderTexture& texture, Array<RenderTarget>& render_targets);
RenderTarget create_render_target(ConcreteRenderer& concrete_renderer, const RenderTexture& texture, Array<RenderTarget>& render_targets);
SingleCreatedResource create_shader(ConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data);
RenderResource create_texture_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, PixelFormat pixel_format, const Vector2u& resolution, void* data);
RenderTexture create_texture(ConcreteRenderer& concrete_renderer, PixelFormat pixel_format, const Vector2u& resolution, void* data);
SingleCreatedResource create_world(Allocator& allocator, const RenderWorldResourceData& data, const RenderTarget& render_target);
void flip(IRendererContext& context);
void move_processed_commads(RendererCommand* command_queue, unsigned command_queue_size, Array<void*>& processed_memory, std::mutex& processed_memory_mutex);
void move_unprocessed_commands(RendererCommand** command_queue, unsigned& command_queue_size, ConcurrentRingBuffer& unprocessed_commands, Allocator& allocator, bool& unprocessed_commands_exist, std::mutex& unprocessed_commands_exist_mutex);
void raise_fence(RenderFence& fence);
void draw(ConcreteRenderer& concrete_renderer, const Vector2u& resolution, RenderResource* resource_table, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const Rect& view);
SingleUpdatedResource update_shader(ConcreteRenderer& concrete_renderer, const RenderResource* resource_table, void* dynamic_data, const ShaderResourceData& data);

}

////////////////////////////////
// Public interface.

Renderer::Renderer(ConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator) :
	_allocator(renderer_allocator), _concrete_renderer(concrete_renderer), _command_queue(nullptr), _command_queue_size(0), _processed_memory(array::create<void*>(_allocator)), _context(nullptr),
	_unprocessed_commands_exist(false), _resource_objects(array::create<RendererResourceObject>(_allocator)), _render_targets(array::create<RenderTarget>(_allocator)), _rendered_worlds(array::create<RenderWorld*>(_allocator))
{
	concurrent_ring_buffer::init(_unprocessed_commands, _allocator, unprocessed_commands_size);
	render_interface::init(_render_interface, render_interface_allocator, _unprocessed_commands, _unprocessed_commands_exist, _unprocessed_commands_exist_mutex, _wait_for_unprocessed_commands_to_exist);
}

Renderer::~Renderer()
{	
	for (unsigned i = 0; i < array::size(_resource_objects); ++i)
	{
		auto& resource_object = _resource_objects[i];
		auto object = render_resource_table::lookup(_resource_table, resource_object.handle).object;

		switch (resource_object.type)
		{
		case RenderResourceData::World:
			render_world::deinit(*(RenderWorld*)object);
			break;
		case RenderResourceData::RenderTarget:
			_allocator.deallocate((RenderTarget*)object);
			break;
		case RenderResourceData::RenderMaterial:
			render_material::deinit(*(RenderMaterial*)object, _allocator);
			break;
		}

		_allocator.deallocate(object);
	}
	
	concurrent_ring_buffer::deinit(_unprocessed_commands);
	array::deinit(_processed_memory);
	array::deinit(_resource_objects);
	array::deinit(_render_targets);
	array::deinit(_rendered_worlds);
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

RenderInterface& Renderer::render_interface()
{
	return _render_interface;
}

const Vector2u& Renderer::resolution() const
{
	return _resolution;
}

void Renderer::run(IRendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_resolution = resolution;
	_thread = std::thread(&Renderer::thread, this);

	// Do stuff here which should happen before anything else.
	render_interface::resize(_render_interface, resolution);
}

void Renderer::stop(Allocator& render_interface_allocator)
{
	_active = false;

	{
		std::lock_guard<std::mutex> unprocessed_commands_exists_lock(_unprocessed_commands_exist_mutex);
		_unprocessed_commands_exist = true;
	}

	_wait_for_unprocessed_commands_to_exist.notify_all();
	_thread.join();
	render_interface::deinit(_render_interface);
	move_unprocessed_commands(&_command_queue, _command_queue_size, _unprocessed_commands, _allocator, _unprocessed_commands_exist, _unprocessed_commands_exist_mutex);
	move_processed_commads(_command_queue, _command_queue_size, _processed_memory, _processed_memory_mutex);
	_allocator.deallocate(_command_queue);
	deallocate_processed_commands(render_interface_allocator);
}


////////////////////////////////
// Implementation.

void Renderer::consume_command_queue()
{
	move_unprocessed_commands(&_command_queue, _command_queue_size, _unprocessed_commands, _allocator, _unprocessed_commands_exist, _unprocessed_commands_exist_mutex);

	for (unsigned i = 0; i < _command_queue_size; ++i)
		execute_command(_command_queue[i]);
		
	move_processed_commads(_command_queue, _command_queue_size, _processed_memory, _processed_memory_mutex);
	_allocator.deallocate(_command_queue);
	_command_queue = nullptr;
	_command_queue_size = 0;
}

SingleCreatedResource single_resource(RenderResourceHandle handle, RenderResource resource)
{
	SingleCreatedResource scr;
	scr.handle = handle;
	scr.resource = resource;
	return scr;
}

CreatedResources copy_single_resource(SingleCreatedResource resource, Allocator& allocator)
{
	CreatedResources cr;
	cr.num = 1;
	cr.handles = (RenderResourceHandle*)allocator.allocate(sizeof(RenderResourceHandle));
	cr.resources = (RenderResource*)allocator.allocate(sizeof(RenderResource));
	cr.handles[0] = resource.handle;
	cr.resources[0] = resource.resource;
	return cr;
}

CreatedResources create_created_resources(unsigned num, Allocator& allocator)
{
	CreatedResources cr;
	cr.num = num;
	cr.handles = (RenderResourceHandle*)allocator.allocate(sizeof(RenderResourceHandle) * num);
	cr.resources = (RenderResource*)allocator.allocate(sizeof(RenderResource) * num);
	return cr;
}

CreatedResources Renderer::create_resources(RenderResourceData::Type type, void* data, void* dynamic_data)
{
	switch(type)
	{
		case RenderResourceData::RenderMaterial: return copy_single_resource(create_material(_allocator, _concrete_renderer, dynamic_data, _resource_table, *(MaterialResourceData*)data), _allocator);
		case RenderResourceData::Shader: return copy_single_resource(create_shader(_concrete_renderer, dynamic_data, *(ShaderResourceData*)data), _allocator);
		case RenderResourceData::Texture: {
			auto texture_resource_data = (TextureResourceData*)data;
			auto texture_bits = memory::pointer_add(dynamic_data, texture_resource_data->texture_data_dynamic_data_offset);
			return copy_single_resource(single_resource(texture_resource_data->handle, create_texture_resource(_concrete_renderer, _allocator, texture_resource_data->pixel_format, texture_resource_data->resolution, texture_bits)), _allocator);
		}
		case RenderResourceData::World: {
			return copy_single_resource(create_world(_allocator, *(RenderWorldResourceData*)data, create_render_target(_concrete_renderer, create_texture(_concrete_renderer, PixelFormat::RGBA, _resolution, 0), _render_targets)), _allocator);
		}
		case RenderResourceData::SpriteRenderer: {
			auto sprite_data = (CreateSpriteRendererData*)data;
			auto& rw = *(RenderWorld*)render_resource_table::lookup(_resource_table, sprite_data->world).object;
			CreatedResources cr = create_created_resources(sprite_data->num, _allocator);
			auto sprite = sprite_renderer_component::create_data_from_buffer(dynamic_data, sprite_data->num);

			for (unsigned i = 0; i < sprite_data->num; ++i)
			{
				auto component = (RenderComponent*)_allocator.allocate(sizeof(RenderComponent));
				component->color = sprite.color[i];
				component->material = sprite.material[i].render_handle;
				component->geometry = sprite.geometry[i];
				render_world::add_component(rw, component);

				cr.handles[i] = sprite.render_handle[i];
				cr.resources[i] = RenderResource(component);
			}

			return cr;
		} break;
		default: assert(!"Unknown render resource type"); return CreatedResources();
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
			draw(_concrete_renderer, _resolution, _resource_table, _rendered_worlds, *(RenderWorld*)render_resource_table::lookup(_resource_table, rwd.render_world).object, rwd.view);
		}
		break;

		// Rename to CreateResource
	case RendererCommand::LoadResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;

			auto created_resources = create_resources(data.type, data.data, dynamic_data);

			for (unsigned i = 0; i < created_resources.num; ++i)
			{
				auto handle = created_resources.handles[i];
				auto resource = created_resources.resources[i];

				assert(resource.type != RenderResource::NotInitialized && "Failed to load resource!");

				// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
				render_resource_table::set(_resource_table, handle, resource);

				// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
				if (resource.type == RenderResource::Object)
					array::push_back(_resource_objects, RendererResourceObject(data.type, handle));
			}

			_allocator.deallocate(created_resources.handles);
			_allocator.deallocate(created_resources.resources);
	
			std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
			array::push_back(_processed_memory, data.data);
		}
		break;

	case RendererCommand::UpdateResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			auto updated_resources = update_resources(data.type, data.data, dynamic_data);

			for (unsigned i = 0; i < updated_resources.num; ++i)
			{
				auto handle = updated_resources.handles[i];
				auto old_resource = updated_resources.old_resources[i];
				auto new_resource = updated_resources.new_resources[i];

				assert(new_resource.type != RenderResource::NotInitialized && "Failed to load resource!");

				if (old_resource.type == RenderResource::Object)
					array::remove(_resource_objects, [&](const RendererResourceObject& rro) { return handle == rro.handle; });

				// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
				render_resource_table::set(_resource_table, handle, new_resource);

				// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
				if (new_resource.type == RenderResource::Object)
					array::push_back(_resource_objects, RendererResourceObject(data.type, handle));
			}

			_allocator.deallocate(updated_resources.handles);
			_allocator.deallocate(updated_resources.new_resources);
			_allocator.deallocate(updated_resources.old_resources);

			std::lock_guard<std::mutex> queue_lock(_processed_memory_mutex);
			array::push_back(_processed_memory, data.data);
		}
		break;

	case RendererCommand::Resize:
		{
			ResizeData& data = *(ResizeData*)command.data;
			_resolution = data.resolution;
			_concrete_renderer.resize(data.resolution, _render_targets);
		}
		break;

	case RendererCommand::CombineRenderedWorlds:
		{
			_concrete_renderer.unset_render_target(_resolution);
			_concrete_renderer.clear();
			_concrete_renderer.combine_rendered_worlds(_rendered_worlds_combining_shader, _rendered_worlds);
			array::clear(_rendered_worlds);
			flip(*_context);
		}
		break;

	case RendererCommand::SetUniformValue:
		{
			const auto& set_uniform_value_data = *(SetUniformValueData*)command.data;
			auto& material = *(RenderMaterial*)render_resource_table::lookup(_resource_table, set_uniform_value_data.material).object;
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

void Renderer::thread()
{
	_context->make_current_for_calling_thread();
	_concrete_renderer.initialize_thread();
	_active = true;

	{
		auto shader_source_option = file::load("rendered_world_combining.shader", _allocator);
		assert(shader_source_option.is_some && "Failed loading rendered world combining shader");
		auto& shader_source = shader_source_option.value;
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

UpdatedResources create_updated_resources(unsigned num, Allocator& allocator)
{
	UpdatedResources ur;
	ur.num = num;
	ur.handles = (RenderResourceHandle*)allocator.allocate(sizeof(RenderResourceHandle) * num);
	ur.old_resources = (RenderResource*)allocator.allocate(sizeof(RenderResource) * num);
	ur.new_resources = (RenderResource*)allocator.allocate(sizeof(RenderResource) * num);
	return ur;
}

UpdatedResources single_update(SingleUpdatedResource resource, Allocator& allocator)
{
	UpdatedResources ur = create_updated_resources(1, allocator);
	ur.handles[0] = resource.handle;
	ur.old_resources[0] = resource.old_resource;
	ur.new_resources[0] = resource.new_resource;
	return ur;
}

UpdatedResources Renderer::update_resources(RenderResourceData::Type type, void* data, void* dynamic_data)
{
	switch(type)
	{
		case RenderResourceData::Shader: return single_update(update_shader(_concrete_renderer, _resource_table, dynamic_data, *(ShaderResourceData*)data), _allocator);
		case RenderResourceData::SpriteRenderer: {
			auto sprite_data = (UpdateSpriteRendererData*)data;
			UpdatedResources ur = create_updated_resources(sprite_data->num, _allocator);

			for (unsigned i = 0; i < sprite_data->num; ++i)
			{
				auto sprite = sprite_renderer_component::create_data_from_buffer(dynamic_data, sprite_data->num);
				auto component = (RenderComponent*)render_resource_table::lookup(_resource_table, sprite.render_handle[i]).object;
				component->color = sprite.color[i];
				component->material = sprite.material[i].render_handle;
				component->geometry = sprite.geometry[i];

				ur.handles[i] = sprite.render_handle[i];
				ur.new_resources[i] = RenderResource(component);
				ur.old_resources[i] = RenderResource(component);
			}

			return ur;
		}
		default: assert(!"Unknown render resource type"); return UpdatedResources();
	}
}

void Renderer::wait_for_unprocessed_commands_to_exist()
{	
	std::unique_lock<std::mutex> unprocessed_commands_exists_lock(_unprocessed_commands_exist_mutex);
	_wait_for_unprocessed_commands_to_exist.wait(unprocessed_commands_exists_lock, [&]{return _unprocessed_commands_exist;});
}

namespace
{

RenderUniform create_uniform(ConcreteRenderer& concrete_renderer, RenderResource shader, const UniformResourceData& uniform_data, const char* name)
{
	auto location = concrete_renderer.get_uniform_location(shader, name);
	auto name_hash = hash_str(name);

	if (uniform_data.automatic_value == uniform::None)
		return RenderUniform(uniform_data.type, name_hash, location);
	else
		return RenderUniform(uniform_data.type, name_hash, location, uniform_data.automatic_value);
}

SingleCreatedResource create_material(Allocator& allocator, ConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResource* resource_table, const MaterialResourceData& data)
{
	auto material = (RenderMaterial*)allocator.allocate(sizeof(RenderMaterial));
	render_material::init(*material, allocator, data.shader);
	auto shader = render_resource_table::lookup(resource_table, data.shader);
	auto uniforms_data = (UniformResourceData*)dynamic_data;
	
	for (unsigned i = 0; i < data.num_uniforms; ++i)
	{
		const auto& uniform_data = uniforms_data[i];
		auto uniform = create_uniform(concrete_renderer, shader, uniform_data, (char*)memory::pointer_add(dynamic_data, uniform_data.name_offset));
		
		if (uniform_data.automatic_value == uniform::None)
		{
			auto value = (void*)memory::pointer_add(dynamic_data, uniform_data.value_offset);

			switch (uniform_data.type)
			{
			case uniform::Float:
				render_uniform::set_value(uniform, allocator, value, sizeof(float));
				break;
			case uniform::Texture1:
			case uniform::Texture2:
			case uniform::Texture3:
				render_uniform::set_value(uniform, allocator, value, sizeof(unsigned));
				break;
			case uniform::Vec4:
				render_uniform::set_value(uniform, allocator, value, sizeof(Vector4));
				break;
			default:
				assert(!"Unkonwn uniform type.");
				break;
			}
		}

		render_material::add_uniform(*material, uniform);
	}

	return single_resource(data.handle, RenderResource(material));
}

RenderResource create_render_target_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, const RenderTexture& texture, Array<RenderTarget>& render_targets)
{
	auto render_target = (RenderTarget*)allocator.allocate(sizeof(RenderTarget));
	*render_target = create_render_target(concrete_renderer, texture, render_targets);
	return RenderResource(render_target);
}

RenderTarget create_render_target(ConcreteRenderer& concrete_renderer, const RenderTexture& texture, Array<RenderTarget>& render_targets)
{
	auto render_target_resource = concrete_renderer.create_render_target(texture);
	RenderTarget rt;
	rt.handle = render_target_resource;
	rt.texture = texture;
	array::push_back(render_targets, rt);
	return rt;
}

SingleCreatedResource create_shader(ConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data)
{
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	return single_resource(data.handle, concrete_renderer.create_shader(vertex_source, fragment_source));
}

RenderResource create_texture_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, PixelFormat pixel_format, const Vector2u& resolution, void* data)
{
	auto texture_resource = concrete_renderer.create_texture(pixel_format, resolution, data);
	RenderTexture* render_texture = (RenderTexture*)allocator.allocate(sizeof(RenderTexture));
	*render_texture = create_texture(concrete_renderer, pixel_format, resolution, data);
	return RenderResource(render_texture);
}

RenderTexture create_texture(ConcreteRenderer& concrete_renderer, PixelFormat pixel_format, const Vector2u& resolution, void* data)
{
	auto texture_resource = concrete_renderer.create_texture(pixel_format, resolution, data);
	RenderTexture render_texture;
	render_texture.pixel_format = pixel_format;
	render_texture.render_handle = texture_resource;
	render_texture.resolution = resolution;
	return render_texture;
}

SingleCreatedResource create_world(Allocator& allocator, const RenderWorldResourceData& data, const RenderTarget& render_target)
{
	auto rw = (RenderWorld*)allocator.allocate(sizeof(RenderWorld));
	render_world::init(*rw, render_target, allocator);
	return single_resource(data.handle, RenderResource(rw));
}

void flip(IRendererContext& context)
{
	context.flip();
}

void move_processed_commads(RendererCommand* command_queue, unsigned command_queue_size, Array<void*>& processed_memory, std::mutex& processed_memory_mutex)
{
	std::lock_guard<std::mutex> queue_lock(processed_memory_mutex);

	for (unsigned i = 0; i < command_queue_size; ++i) {
		const auto& command = command_queue[i];
		auto dont_free = command.type == RendererCommand::Fence;

		if (dont_free)
			continue;

		array::push_back(processed_memory, command.data);
		array::push_back(processed_memory, command.dynamic_data);
	}
}

void move_unprocessed_commands(RendererCommand** command_queue, unsigned& command_queue_size, ConcurrentRingBuffer& unprocessed_commands, Allocator& allocator, bool& unprocessed_commands_exist, std::mutex& unprocessed_commands_exist_mutex)
{
	std::lock_guard<std::mutex> lock(unprocessed_commands_exist_mutex);
	auto consumed = concurrent_ring_buffer::consume_all(unprocessed_commands, allocator);
	unprocessed_commands_exist = false;
	*command_queue = (RendererCommand*)consumed.data;
	command_queue_size = consumed.size / sizeof(RendererCommand);
}

void raise_fence(RenderFence& fence)
{
	std::lock_guard<std::mutex> fence_lock(fence.mutex);
	fence.processed = true;
	fence.fence_processed.notify_all();
}

void draw(ConcreteRenderer& concrete_renderer, const Vector2u& resolution, RenderResource* resource_table, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const Rect& view)
{
	render_world::sort(render_world);
	concrete_renderer.set_render_target(resolution, render_world.render_target.handle);
	concrete_renderer.clear();
	concrete_renderer.draw(view, render_world, resolution, resource_table);
	array::push_back(rendered_worlds, &render_world);
}

SingleUpdatedResource update_shader(ConcreteRenderer& concrete_renderer, const RenderResource* resource_table, void* dynamic_data, const ShaderResourceData& data)
{
	RenderResource old_resource = render_resource_table::lookup(resource_table, data.handle);
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	RenderResource new_resource = concrete_renderer.update_shader(old_resource, vertex_source, fragment_source);
	SingleUpdatedResource sur;
	sur.handle = data.handle;
	sur.old_resource = old_resource;
	sur.new_resource = new_resource;
	return sur;
}

} // implementation

} // namespace bowtie
