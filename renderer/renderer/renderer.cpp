#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <engine/render_fence.h>
#include <engine/shader_utils.h>
#include <engine/entity/components/sprite_renderer_component.h>
#include <foundation/array.h>
#include <foundation/file.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>
#include "render_material.h"
#include "render_world.h"
#include "render_target.h"
#include "render_texture.h"
#include "render_component.h"
#include <engine/material.h>

namespace bowtie
{

namespace internal
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
	cr.handles = (RenderResourceHandle*)allocator.alloc(sizeof(RenderResourceHandle));
	cr.resources = (RenderResource*)allocator.alloc(sizeof(RenderResource));
	cr.handles[0] = resource.handle;
	cr.resources[0] = resource.resource;
	return cr;
}

CreatedResources create_created_resources(unsigned num, Allocator& allocator)
{
	CreatedResources cr;
	cr.num = num;
	cr.handles = (RenderResourceHandle*)allocator.alloc(sizeof(RenderResourceHandle) * num);
	cr.resources = (RenderResource*)allocator.alloc(sizeof(RenderResource) * num);
	return cr;
}

RenderUniform create_uniform(ConcreteRenderer& concrete_renderer, RenderResource shader, const UniformResourceData& uniform_data, const char* name)
{
	auto location = concrete_renderer.get_uniform_location(shader, name);
	auto name_hash = hash_str(name);

	RenderUniform ru = {};
	ru.type = uniform_data.type;
	ru.name = name_hash;
	ru.location = location;

	if (uniform_data.automatic_value != uniform::None)
		ru.automatic_value = uniform_data.automatic_value;

	return ru;
}

SingleCreatedResource create_material(Allocator& allocator, ConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResource* resource_table, const MaterialResourceData& data)
{
	auto material = (RenderMaterial*)allocator.alloc(sizeof(RenderMaterial));
	render_material::init(material, &allocator, data.num_uniforms, data.shader);
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
				render_uniform::set_value(&uniform, &allocator, value, sizeof(float));
				break;
			case uniform::Texture1:
			case uniform::Texture2:
			case uniform::Texture3:
				render_uniform::set_value(&uniform, &allocator, value, sizeof(unsigned));
				break;
			case uniform::Vec4:
				render_uniform::set_value(&uniform, &allocator, value, sizeof(Vector4));
				break;
			default:
				assert(!"Unkonwn uniform type.");
				break;
			}
		}

		material->uniforms[i] = uniform;
	}

	return single_resource(data.handle, render_resource::create_object(material));
}

RenderTarget* find_free_render_target_slot(RenderTarget* render_targets)
{
	for (unsigned i = 0; i < renderer::max_render_targets; ++i)
	{
		if (render_targets[i].handle.type == RenderResourceType::NotInitialized)
			return render_targets + i;
	}

	assert("Out of render targets");
	return nullptr;
}

RenderTarget create_render_target(ConcreteRenderer& concrete_renderer, const RenderTexture& texture, RenderTarget* render_targets)
{
	auto render_target_resource = concrete_renderer.create_render_target(&texture);
	auto rt = find_free_render_target_slot(render_targets);
	rt->handle = render_target_resource;
	rt->texture = texture;
	return *rt;
}

RenderResource create_render_target_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, const RenderTexture& texture, RenderTarget* render_targets)
{
	auto render_target = (RenderTarget*)allocator.alloc(sizeof(RenderTarget));
	*render_target = create_render_target(concrete_renderer, texture, render_targets);
	return render_resource::create_object(render_target);
}

SingleCreatedResource create_shader(ConcreteRenderer& concrete_renderer, void* dynamic_data, const ShaderResourceData& data)
{
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	return single_resource(data.handle, concrete_renderer.create_shader(vertex_source, fragment_source));
}

RenderTexture create_texture(ConcreteRenderer& concrete_renderer, PixelFormat pixel_format, const Vector2u& resolution, void* data)
{
	auto texture_resource = concrete_renderer.create_texture(pixel_format, &resolution, data);
	RenderTexture render_texture;
	render_texture.pixel_format = pixel_format;
	render_texture.render_handle = texture_resource;
	render_texture.resolution = resolution;
	return render_texture;
}

RenderResource create_texture_resource(ConcreteRenderer& concrete_renderer, Allocator& allocator, PixelFormat pixel_format, const Vector2u& resolution, void* data)
{
	RenderTexture* render_texture = (RenderTexture*)allocator.alloc(sizeof(RenderTexture));
	*render_texture = create_texture(concrete_renderer, pixel_format, resolution, data);
	return render_resource::create_object(render_texture);
}

SingleCreatedResource create_world(Allocator& allocator, const RenderWorldResourceData& data, const RenderTarget& render_target)
{
	auto rw = (RenderWorld*)allocator.alloc(sizeof(RenderWorld));
	render_world::init(*rw, render_target, allocator);
	return single_resource(data.handle, render_resource::create_object(rw));
}

void flip(RendererContext* context, PlatformRendererContextData* platform_data)
{
	context->flip(platform_data);
}

void raise_fence(RenderFence& fence)
{
	std::lock_guard<std::mutex> fence_lock(fence.mutex);
	fence.processed = true;
	fence.fence_processed.notify_all();
}

void draw(ConcreteRenderer& concrete_renderer, const Vector2u& resolution, RenderResource* resource_table, RenderWorld** rendered_worlds, unsigned* num_rendered_worlds, RenderWorld& render_world, const Rect& view, float time)
{
	render_world::sort(render_world);
	concrete_renderer.set_render_target(&resolution, render_world.render_target.handle);
	concrete_renderer.clear();
	concrete_renderer.draw(&view, &render_world, &resolution, time, resource_table);
	assert(*num_rendered_worlds < renderer::max_rendered_worlds);
	rendered_worlds[*num_rendered_worlds] = &render_world;
	++(*num_rendered_worlds);
}

SingleUpdatedResource update_shader(ConcreteRenderer& concrete_renderer, const RenderResource* resource_table, void* dynamic_data, const ShaderResourceData& data)
{
	RenderResource old_resource = render_resource_table::lookup(resource_table, data.handle);
	const char* vertex_source = (const char*)memory::pointer_add(dynamic_data, data.vertex_shader_source_offset);
	const char* fragment_source = (const char*)memory::pointer_add(dynamic_data, data.fragment_shader_source_offset);
	RenderResource new_resource = concrete_renderer.update_shader(&old_resource, vertex_source, fragment_source);
	SingleUpdatedResource sur;
	sur.handle = data.handle;
	sur.old_resource = old_resource;
	sur.new_resource = new_resource;
	return sur;
}

CreatedResources create_resources(Renderer& r, RenderResourceData::Type type, void* data, void* dynamic_data)
{
	switch (type)
	{
	case RenderResourceData::RenderMaterial: return copy_single_resource(create_material(*r.allocator, r._concrete_renderer, dynamic_data, r.resource_table, *(MaterialResourceData*)data), *r.allocator);
	case RenderResourceData::Shader: return copy_single_resource(create_shader(r._concrete_renderer, dynamic_data, *(ShaderResourceData*)data), *r.allocator);
	case RenderResourceData::Texture: {
		auto texture_resource_data = (TextureResourceData*)data;
		auto texture_bits = memory::pointer_add(dynamic_data, texture_resource_data->texture_data_dynamic_data_offset);
		return copy_single_resource(single_resource(texture_resource_data->handle, create_texture_resource(r._concrete_renderer, *r.allocator, texture_resource_data->pixel_format, texture_resource_data->resolution, texture_bits)), *r.allocator);
	}
	case RenderResourceData::World: {
		return copy_single_resource(create_world(*r.allocator, *(RenderWorldResourceData*)data, create_render_target(r._concrete_renderer, create_texture(r._concrete_renderer, PixelFormat::RGBA, r.resolution, 0), r._render_targets)), *r.allocator);
	}
	case RenderResourceData::SpriteRenderer: {
		auto sprite_data = (CreateSpriteRendererData*)data;
		auto& rw = *(RenderWorld*)render_resource_table::lookup(r.resource_table, sprite_data->world).object;
		CreatedResources cr = create_created_resources(sprite_data->num, *r.allocator);
		auto sprite = sprite_renderer_component::create_data_from_buffer(dynamic_data, sprite_data->num);

		for (unsigned i = 0; i < sprite_data->num; ++i)
		{
			auto component = (RenderComponent*)r.allocator->alloc(sizeof(RenderComponent));
			component->color = sprite.color[i];
			component->material = sprite.material[i].render_handle;
			component->geometry = sprite.geometry[i];
			component->depth = sprite.depth[i];
			render_world::add_component(rw, component);

			cr.handles[i] = sprite.render_handle[i];
			cr.resources[i] = render_resource::create_object(component);
		}

		return cr;
	} break;
	default: assert(!"Unknown render resource type"); return CreatedResources();
	}
}

UpdatedResources create_updated_resources(unsigned num, Allocator& allocator)
{
	UpdatedResources ur;
	ur.num = num;
	ur.handles = (RenderResourceHandle*)allocator.alloc(sizeof(RenderResourceHandle) * num);
	ur.old_resources = (RenderResource*)allocator.alloc(sizeof(RenderResource) * num);
	ur.new_resources = (RenderResource*)allocator.alloc(sizeof(RenderResource) * num);
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

UpdatedResources update_resources(Renderer& r, RenderResourceData::Type type, void* data, void* dynamic_data)
{
	switch (type)
	{
		case RenderResourceData::Shader: return single_update(update_shader(r._concrete_renderer, r.resource_table, dynamic_data, *(ShaderResourceData*)data), *r.allocator);
		case RenderResourceData::SpriteRenderer: {
			auto sprite_data = (UpdateSpriteRendererData*)data;
			UpdatedResources ur = create_updated_resources(sprite_data->num, *r.allocator);

			for (unsigned i = 0; i < sprite_data->num; ++i)
			{
				auto sprite = sprite_renderer_component::create_data_from_buffer(dynamic_data, sprite_data->num);
				auto component = (RenderComponent*)render_resource_table::lookup(r.resource_table, sprite.render_handle[i]).object;
				component->color = sprite.color[i];
				component->material = sprite.material[i].render_handle;
				component->geometry = sprite.geometry[i];
				component->depth = sprite.depth[i];

				ur.handles[i] = sprite.render_handle[i];
				ur.new_resources[i] = render_resource::create_object(component);
				ur.old_resources[i] = render_resource::create_object(component);
			}

			return ur;
		}
		default: assert(!"Unknown render resource type"); return UpdatedResources();
	}
}

void execute_command(Renderer& r, const RendererCommand& command)
{
	switch (command.type)
	{
		case RendererCommand::Fence:
			raise_fence(*(RenderFence*)command.data);
			break;

		case RendererCommand::RenderWorld:
		{
			RenderWorldData& rwd = *(RenderWorldData*)command.data;
			draw(r._concrete_renderer, r.resolution, r.resource_table, r._rendered_worlds, &r.num_rendered_worlds, *(RenderWorld*)render_resource_table::lookup(r.resource_table, rwd.render_world).object, rwd.view, rwd.time);
		} break;

		// Rename to CreateResource
		case RendererCommand::LoadResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;

			auto created_resources = create_resources(r, data.type, data.data, dynamic_data);

			for (unsigned i = 0; i < created_resources.num; ++i)
			{
				auto handle = created_resources.handles[i];
				auto resource = created_resources.resources[i];

				assert(resource.type != RenderResourceType::NotInitialized && "Failed to load resource!");

				// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
				render_resource_table::set(r.resource_table, handle, resource);

				// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
				if (resource.type == RenderResourceType::Object)
					r._resource_objects[handle] = RendererResourceObject(data.type, handle);
			}

			r.allocator->dealloc(created_resources.handles);
			r.allocator->dealloc(created_resources.resources);

			std::lock_guard<std::mutex> queue_lock(r._processed_memory_mutex);
			array::push_back(r._processed_memory, data.data);
		} break;

		case RendererCommand::UpdateResource:
		{
			RenderResourceData& data = *(RenderResourceData*)command.data;
			void* dynamic_data = command.dynamic_data;
			auto updated_resources = update_resources(r, data.type, data.data, dynamic_data);

			for (unsigned i = 0; i < updated_resources.num; ++i)
			{
				auto handle = updated_resources.handles[i];
				auto old_resource = updated_resources.old_resources[i];
				auto new_resource = updated_resources.new_resources[i];

				assert(new_resource.type != RenderResourceType::NotInitialized && "Failed to load resource!");

				if (old_resource.type == RenderResourceType::Object)
					memset(r._resource_objects + handle, 0, sizeof(RendererResourceObject));

				// Map handle from outside of renderer (RenderResourceHandle) to internal handle (RenderResource).
				render_resource_table::set(r.resource_table, handle, new_resource);

				// Save dynamically allocated render resources in _resource_objects for deallocation on shutdown.
				if (new_resource.type == RenderResourceType::Object)
					r._resource_objects[handle] = RendererResourceObject(data.type, handle);
			}

			r.allocator->dealloc(updated_resources.handles);
			r.allocator->dealloc(updated_resources.new_resources);
			r.allocator->dealloc(updated_resources.old_resources);

			std::lock_guard<std::mutex> queue_lock(r._processed_memory_mutex);
			array::push_back(r._processed_memory, data.data);
		} break;

		case RendererCommand::Resize:
		{
			auto data = (ResizeData*)command.data;
			r.resolution = data->resolution;
			r._concrete_renderer.resize(&data->resolution, r._render_targets);
		} break;

		case RendererCommand::CombineRenderedWorlds:
		{
			r._concrete_renderer.unset_render_target(&r.resolution);
			r._concrete_renderer.clear();
			r._concrete_renderer.combine_rendered_worlds(r._rendered_worlds_combining_shader, r._rendered_worlds, r.num_rendered_worlds);
			r.num_rendered_worlds = 0;
			flip(&r._context, r._context_data);
		} break;

		case RendererCommand::SetUniformValue:
		{
			const auto& set_uniform_value_data = *(SetUniformValueData*)command.data;
			auto material = (RenderMaterial*)render_resource_table::lookup(r.resource_table, set_uniform_value_data.material).object;
			switch (set_uniform_value_data.type)
			{
			case uniform::Float:
				render_material::set_uniform_float_value(material, r.allocator, set_uniform_value_data.uniform_name, *(float*)command.dynamic_data);
				break;
			default:
				assert(!"Unknown uniform type");
				break;
			}
		} break;

		default:
			assert(!"Command not implemented!");
			break;
	}
}

void consume_command_queue(Renderer& r)
{
	{
		std::lock_guard<std::mutex> lock(r._unprocessed_commands_exist_mutex);
		r._unprocessed_commands_exist = false;
	}

	auto command = (RendererCommand*)concurrent_ring_buffer::peek(r._unprocessed_commands);

	while (command != nullptr)
	{
		execute_command(r, *command);

		{
			std::lock_guard<std::mutex> queue_lock(r._processed_memory_mutex);
			auto dont_free = command->type == RendererCommand::Fence;

			if (!dont_free)
			{
				array::push_back(r._processed_memory, command->data);
				array::push_back(r._processed_memory, command->dynamic_data);
			}
		}

		concurrent_ring_buffer::consume_one(r._unprocessed_commands);
		command = (RendererCommand*)concurrent_ring_buffer::peek(r._unprocessed_commands);
	}
}

void wait_for_unprocessed_commands_to_exist(Renderer& r)
{
	std::unique_lock<std::mutex> unprocessed_commands_exists_lock(r._unprocessed_commands_exist_mutex);
	r._wait_for_unprocessed_commands_to_exist.wait(unprocessed_commands_exists_lock, [&]{return r._unprocessed_commands_exist; });
}

void thread(Renderer* renderer)
{
	auto& r = *renderer;
	r._context.make_current_for_calling_thread(r._context_data);
	r._concrete_renderer.initialize_thread();
	r.active = true;

	{
		auto shader_source_option = file::load("rendered_world_combining.shader", r.allocator);
		assert(shader_source_option.is_some && "Failed loading rendered world combining shader");
		auto& shader_source = shader_source_option.value;
		auto split_shader = shader_utils::split_shader(&shader_source, r.allocator);
		r._rendered_worlds_combining_shader = r._concrete_renderer.create_shader(split_shader.vertex_source, split_shader.fragment_source);
		r.allocator->dealloc(shader_source.data);
		r.allocator->dealloc(split_shader.vertex_source);
		r.allocator->dealloc(split_shader.fragment_source);
	}

	while (r.active)
	{
		wait_for_unprocessed_commands_to_exist(r);
		consume_command_queue(r);
	}
}

} // namespace internal

namespace renderer
{

void init(Renderer& r, const ConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, const RendererContext* context)
{
	r.allocator = &renderer_allocator;
	r.active = false;
	r._concrete_renderer = concrete_renderer;
	array::init(r._processed_memory, *r.allocator);
	memset(r._render_targets, 0, sizeof(RenderTarget) * max_render_targets);
	memset(r._resource_objects, 0, sizeof(RendererResourceObject) * render_resource_handle::num);
	r._unprocessed_commands_exist = false;
	r.num_rendered_worlds = 0;
	r._context = *context;
	const auto unprocessed_commands_num = 64000;
	concurrent_ring_buffer::init(r._unprocessed_commands, *r.allocator, unprocessed_commands_num, sizeof(RendererCommand));
	render_interface::init(&r.render_interface, &render_interface_allocator, &r._unprocessed_commands, &r._unprocessed_commands_exist, &r._unprocessed_commands_exist_mutex, &r._wait_for_unprocessed_commands_to_exist);
}

void deinit(Renderer& r)
{	
	for (unsigned i = 0; i < render_resource_handle::num; ++i)
	{
		auto& resource_object = r._resource_objects[i];

		if (resource_object.handle == handle_not_initialized)
			continue;

		auto object = render_resource_table::lookup(r.resource_table, resource_object.handle).object;

		switch (resource_object.type)
		{
		case RenderResourceData::World:
			render_world::deinit(*(RenderWorld*)object);
			break;
		case RenderResourceData::RenderTarget:
			r.allocator->dealloc((RenderTarget*)object);
			break;
		case RenderResourceData::RenderMaterial:
			render_material::deinit((RenderMaterial*)object, r.allocator);
			break;
		}

		r.allocator->dealloc(object);
	}
	
	concurrent_ring_buffer::deinit(r._unprocessed_commands);
	array::deinit(r._processed_memory);
}

void deallocate_processed_commands(Renderer& r, Allocator& render_interface_allocator)
{
	std::lock_guard<std::mutex> queue_lock(r._processed_memory_mutex);

	for (unsigned i = 0; i < array::size(r._processed_memory); ++i)
	{
		void* ptr = r._processed_memory[i];
		render_interface_allocator.dealloc(ptr);
	}

	array::clear(r._processed_memory);
}

void run(Renderer& r, PlatformRendererContextData* context_data, const Vector2u& resolution)
{
	r._context_data = context_data;
	r.resolution = resolution;
	r._thread = std::thread(&internal::thread, &r);

	// Do stuff here which should happen before anything else.
	render_interface::resize(&r.render_interface, &resolution);
}

void stop(Renderer& r, Allocator& render_interface_allocator)
{
	r.active = false;

	{
		std::lock_guard<std::mutex> unprocessed_commands_exists_lock(r._unprocessed_commands_exist_mutex);
		r._unprocessed_commands_exist = true;
	}

	r._wait_for_unprocessed_commands_to_exist.notify_all();
	r._thread.join();
	deallocate_processed_commands(r, render_interface_allocator);
}

} // namespace renderer

} // namespace bowtie
