#include "renderer.h"

#include <cassert>

#include <foundation/array.h>
#include <foundation/queue.h>

#include "render_fence.h"
#include "render_drawable.h"
#include "render_world.h"

namespace bowtie
{

Renderer::Renderer(Allocator& allocator) : _command_queue(allocator), _free_handles(allocator), _allocator(allocator), _unprocessed_commands(allocator),
	_render_interface(*this, allocator), _context(nullptr), _is_setup(false), _resource_objects(allocator), _render_targets(allocator), _rendered_worlds(allocator)
{
	array::set_capacity(_free_handles, num_handles);

	for(unsigned handle = num_handles; handle > 0; --handle)
		array::push_back(_free_handles, ResourceHandle(handle));
}

Renderer::~Renderer()
{
	for (unsigned i = 0; i < array::size(_resource_objects); ++i)
	{
		auto& resource_object = _resource_objects[i];
		switch (resource_object.type)
		{
		case RenderResourceData::World:
			MAKE_DELETE(_allocator, RenderWorld, (RenderWorld*)resource_object.handle.render_object);
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

	notify_command_queue_populated();
}

RenderResourceHandle Renderer::create_drawable(DrawableResourceData& drawable_data)
{
	RenderDrawable& drawable = *(RenderDrawable*)_allocator.allocate(sizeof(RenderDrawable));

	drawable.texture = drawable_data.texture;
	drawable.model = drawable_data.model;
	drawable.shader = drawable_data.shader;
	drawable.geometry = drawable_data.geometry;
	drawable.num_vertices = drawable_data.num_vertices;

	RenderResourceHandle drawable_handle = &drawable;

	RenderWorld& render_world = *(RenderWorld*)lookup_resource_object(drawable_data.render_world.handle).render_object;
	render_world.add_drawable(drawable_handle);

	return drawable_handle;
}

RenderResourceHandle Renderer::create_world()
{
	return MAKE_NEW(_allocator, RenderWorld, _allocator, create_render_target());
}

void Renderer::create_resource(RenderResourceData& render_resource, void* dynamic_data)
{
	RenderResourceHandle handle;

	switch(render_resource.type)
	{
	case RenderResourceData::Shader:
		handle = load_shader(*(ShaderResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Texture:
		handle = load_texture(*(TextureResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Drawable:
		handle = create_drawable(*(DrawableResourceData*)render_resource.data); break;
	case RenderResourceData::World:
		handle = create_world(); break;
	case RenderResourceData::Geometry:
		handle = load_geometry(*(GeometryResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Target:
		{
			auto render_target = create_render_target();
			handle = render_target;
			array::push_back(_render_targets, render_target);
		}
		break;
	default:
		assert(!"Unknown render resource type"); return;
	}
	_allocator.deallocate(render_resource.data);

	assert(handle.type != RenderResourceHandle::NotInitialized && "Failed to load resource!");
						
	// Map handle from outside of renderer (ResourceHandle) to internal handle (RenderResourceHandle).
	_resource_lut[render_resource.handle.handle] = handle;

	if (handle.type == RenderResourceHandle::Object)
	{
		auto rro = RendererResourceObject();
		rro.handle = handle;
		rro.type = render_resource.type;
		array::push_back(_resource_objects, rro);
	}
}

void Renderer::consume_command_queue()
{
	move_unprocessed_commands();

	while (queue::size(_command_queue) > 0)
	{
		RendererCommand& command = _command_queue[0];
		queue::pop_front(_command_queue);
		
		bool clear_command_data = true;

		switch(command.type)
		{

		case RendererCommand::Fence:
			{
				RenderFence& fence = *(RenderFence*)command.data;

				{
					std::lock_guard<std::mutex> fence_lock(fence.mutex);
					fence.processed = true;
					fence.fence_processed.notify_all();
					clear_command_data = false;
				}
			}
			break;
			
		case RendererCommand::RenderWorld:
			{
				RenderWorldData& rwd = *(RenderWorldData*)command.data;
				render_world(rwd.view, rwd.render_world);
			}
			break;

		case RendererCommand::LoadResource:
			{
				RenderResourceData& data = *(RenderResourceData*)command.data;
				create_resource(data, command.dynamic_data);
			}
			break;

		case RendererCommand::Resize:
			{
				ResizeData& data = *(ResizeData*)command.data;
				_resolution = data.resolution;
				resize(data.resolution, _render_targets);
			}
			break;
		case RendererCommand::DrawableStateReflection:
			{
				drawable_state_reflection(*(DrawableStateReflectionData*)command.data);
			}
			break;
		case RendererCommand::DrawableGeometryReflection:
			{
				update_geometry(*(DrawableGeometryReflectionData*)command.data, command.dynamic_data);
			}
			break;
		case RendererCommand::CombineRenderedWorlds:
			{
				combine_rendered_worlds(_rendered_worlds);
				array::clear(_rendered_worlds);
			}
			break;
		default:
			assert(!"Command not implemented!");
			break;
		}	

		if(clear_command_data)
		{
			_allocator.deallocate(command.data);
			_allocator.deallocate(command.dynamic_data);
		}
	}
}

void Renderer::drawable_state_reflection(const DrawableStateReflectionData& data)
{
	RenderDrawable& drawable = *(RenderDrawable*)lookup_resource_object(data.drawble.handle).render_object;
	drawable.model = data.model;
}

ResourceHandle Renderer::create_handle()
{
	assert(array::any(_free_handles) && "Out of render resource handles!");

	ResourceHandle handle = array::back(_free_handles);
	array::pop_back(_free_handles);

	return handle;
}

void Renderer::notify_command_queue_populated()
{
	{
		std::lock_guard<std::mutex>	populated_lock(_command_queue_populated_mutex);
		_command_queue_populated = true;
		_wait_for_command_queue_populated.notify_all();
	}
}

void Renderer::move_unprocessed_commands()
{
	std::lock_guard<std::mutex> queue_lock(_unprocessed_commands_mutex);

	for(unsigned i = 0; i < array::size(_unprocessed_commands); ++i)
		queue::push_back(_command_queue, _unprocessed_commands[i]);

	array::clear(_unprocessed_commands);
}

void Renderer::render_world(const View& view, ResourceHandle render_world_handle)
{
	RenderWorld& render_world = *(RenderWorld*)lookup_resource_object(render_world_handle.handle).render_object;
	set_render_target(*(RenderTarget*)render_world.render_target().render_object);
	clear();
	draw(view, render_world_handle);
	array::push_back(_rendered_worlds, render_world_handle);
}

RenderResourceHandle Renderer::lookup_resource_object(ResourceHandle handle) const
{
	assert(handle.type == ResourceHandle::Handle && "Resource is not of handle-type");
	assert(handle.handle < num_handles && "Handle is out of range");

	return _resource_lut[handle.handle];
}

void Renderer::run(RendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_rendering_thread = std::thread(&Renderer::run_thread, this);

	// Do stuff here which should happen before anything else.
	_render_interface.resize(resolution);

	_is_setup = true;
}

}