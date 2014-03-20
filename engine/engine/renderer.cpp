#include "renderer.h"

#include <cassert>

#include <foundation/array.h>
#include <foundation/queue.h>

#include "render_fence.h"

namespace bowtie
{

Renderer::Renderer(Allocator& allocator) : _command_queue(allocator), _allocator(allocator), _unprocessed_commands(allocator), _render_interface(*this, allocator)
{
}

Renderer::~Renderer()
{
}

void Renderer::add_renderer_command(RendererCommand& command)
{
	{
		std::lock_guard<std::mutex> queue_lock(_unprocessed_commands_mutex);
		array::push_back(_unprocessed_commands, command);
	}

	notify_command_queue_populated();
}

void Renderer::consume_command_queue()
{
	move_unprocessed_commands();

	while(queue::size(_command_queue) > 0)
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
					std::lock_guard<std::mutex> lk(fence.mutex);
					fence.processed = true;
					fence.fence_processed.notify_all();
					clear_command_data = false;
				}
			}
			break;
			
		case RendererCommand::RenderWorld:
			{
				render_world();
			}
			break;

		default:
			assert(!"Unknown renderer command");
			break;
		}	

		if(clear_command_data)
			_allocator.deallocate(command.data);
	}
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

void Renderer::render_world()
{
	clear();
	flip();
}

}