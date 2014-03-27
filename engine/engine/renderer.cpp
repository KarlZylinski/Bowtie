#include "renderer.h"

#include <cassert>

#include <foundation/array.h>
#include <foundation/queue.h>

#include "render_fence.h"

namespace bowtie
{

Renderer::Renderer(Allocator& allocator) : _command_queue(allocator), _free_handles(allocator), _allocator(allocator), _unprocessed_commands(allocator),
	_render_interface(*this, allocator), _context(nullptr), _is_setup(false)
{
	array::set_capacity(_free_handles, num_handles);

	for(unsigned handle = num_handles; handle > 0; --handle)
		array::push_back(_free_handles, ResourceHandle(handle));
}

Renderer::~Renderer()
{
}

void Renderer::add_renderer_command(const RendererCommand& command)
{
	{
		std::lock_guard<std::mutex> queue_lock(_unprocessed_commands_mutex);
		array::push_back(_unprocessed_commands, command);
	}

	notify_command_queue_populated();
}


void Renderer::load_resource(RenderResourceData& render_resource, void* dynamic_data)
{
	RenderResourceHandle handle;

	switch(render_resource.type)
	{
	case RenderResourceData::Shader:
		handle = load_shader(*(ShaderResourceData*)render_resource.data, dynamic_data); break;
	case RenderResourceData::Texture:
		handle = load_BMP(*(TextureResourceData*)render_resource.data, dynamic_data); break;
	default:
		assert(!"Unknown render resource type"); return;
	}
	_allocator.deallocate(render_resource.data);

	assert(handle.type != RenderResourceHandle::NotInitialized && "Failed to load resource!");
						
	// Map handle from outside of renderer (ResourceHandle) to internal handle (RenderResourceHandle).
	_resource_lut[render_resource.handle.handle] = handle;
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
				render_world(rwd.view);
			}
			break;

		case RendererCommand::LoadResource:
			{
				RenderResourceData& data = *(RenderResourceData*)command.data;
				load_resource(data, command.dynamic_data);
			}
			break;

		case RendererCommand::Resize:
			{
				ResizeData& data = *(ResizeData*)command.data;
				_resolution = data.resolution;
				resize(data.resolution);
			}
			break;
		case RendererCommand::SetUpSpriteRenderingQuad:
			{
				_sprite_rendering_quad_handle = create_handle();
				_resource_lut[_sprite_rendering_quad_handle.handle] = set_up_sprite_rendering_quad();
			}
			break;
		default:
			assert(!"Command not implemented!");
			break;
		}	

		if(clear_command_data) {
			_allocator.deallocate(command.data);
			_allocator.deallocate(command.dynamic_data);
		}
	}
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

void Renderer::render_world(const View& view)
{
	assert(_sprite_rendering_quad_handle.type != ResourceHandle::NotInitialized && "_sprite_rendering_quad not initialized. Please set it to a handle of a 1x1 quad which will be used for drawing sprites by implementing Rendering.set_up_sprite_rendering_quad() correctly.");

	clear();

	test_draw(view);

	flip();
}

RenderResourceHandle Renderer::lookup_resource_object(ResourceHandle handle) const
{
	return _resource_lut[handle.handle];
}

void Renderer::run(RendererContext* context, const Vector2u& resolution)
{
	_context = context;
	_rendering_thread = std::thread(&Renderer::run_thread, this);

	// Do stuff here which should happen before anything else.
	_render_interface.resize(resolution);
	_render_interface.dispatch(_render_interface.create_command(RendererCommand::SetUpSpriteRenderingQuad));

	_is_setup = true;
}

}