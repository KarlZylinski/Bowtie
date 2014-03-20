#pragma once

#include <condition_variable>
#include <mutex>

#include <foundation/collection_types.h>

#include "render_interface.h"
#include "renderer_command.h"

namespace bowtie
{

class Renderer
{
public:
	Renderer(Allocator& allocator);
	virtual ~Renderer();
	
	bool active() const { return _active; }
	void add_renderer_command(RendererCommand& command);
	virtual void clear() = 0;
	virtual void flip() = 0;
	void consume_command_queue();
	RenderInterface& render_interface() { return _render_interface; }
	virtual void run_render_thread() = 0;
	
protected:
	void move_unprocessed_commands();
	void set_active(bool active) { _active = active; }
	
	Queue<RendererCommand> _command_queue;
	bool _command_queue_populated;
	std::mutex _command_queue_populated_mutex;
	std::condition_variable _wait_for_command_queue_populated;
	void notify_command_queue_populated();

private:
	bool _active;
	Allocator& _allocator;
	void render_world();
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	RenderInterface _render_interface;
};

}