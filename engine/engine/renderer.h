#pragma once

#include <condition_variable>
#include <mutex>

#include <foundation/collection_types.h>

#include "internal_render_resource_handle.h"
#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"

namespace bowtie
{

class Renderer
{
public:
	static const unsigned num_handles = 4000;

	Renderer(Allocator& allocator);
	virtual ~Renderer();
	
	bool active() const { return _active; }
	void add_renderer_command(const RendererCommand& command);
	void load_resource(RenderResourceData& render_resource, void* dynamic_data);
	void consume_command_queue();
	RenderResourceHandle create_handle();
	RenderInterface& render_interface() { return _render_interface; }

	// Renderer API specific
	virtual void clear() = 0;
	virtual void flip() = 0;
	virtual void run_render_thread() = 0;
	virtual InternalRenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data) = 0;
	
protected:
	void move_unprocessed_commands();
	void set_active(bool active) { _active = active; }
	
	Queue<RendererCommand> _command_queue;
	bool _command_queue_populated;
	std::mutex _command_queue_populated_mutex;
	std::condition_variable _wait_for_command_queue_populated;
	void notify_command_queue_populated();
	Array<RenderResourceHandle> _free_handles;
	InternalRenderResourceHandle _resource_lut[num_handles];

private:
	bool _active;
	Allocator& _allocator;
	void render_world();
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	RenderInterface _render_interface;
};

}