#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <foundation/collection_types.h>

#include "render_resource_handle.h"
#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"
#include "renderer_context.h"

namespace bowtie
{

class Renderer
{
public:
	static const unsigned num_handles = 4000;

	Renderer(Allocator& allocator);
	virtual ~Renderer();
	
	bool active() const { return _active; }
	const Vector2u& resolution() const { return _resolution; }
	void add_renderer_command(const RendererCommand& command);
	void load_resource(RenderResourceData& render_resource, void* dynamic_data);
	void consume_command_queue();
	ResourceHandle create_handle();
	RenderInterface& render_interface() { return _render_interface; }
	void run(RendererContext* context, const Vector2u& resolution);
	RenderResourceHandle lookup_resource_object(ResourceHandle handle) const;
	bool is_setup() const { return _is_setup; }

	// Renderer API specific
	virtual void test_draw(const View& view) = 0;
	virtual void clear() = 0;
	virtual void flip() = 0;
	virtual void resize(const Vector2u& size) = 0;
	virtual RenderResourceHandle set_up_sprite_rendering_quad() = 0;
	virtual RenderResourceHandle load_BMP(TextureResourceData& trd, void* dynamic_data) = 0;
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data) = 0;

protected:
	virtual void run_thread() = 0;
	void move_unprocessed_commands();
	void set_active(bool active) { _active = active; }
		
	Queue<RendererCommand> _command_queue;
	bool _command_queue_populated;
	std::mutex _command_queue_populated_mutex;
	std::condition_variable _wait_for_command_queue_populated;
	void notify_command_queue_populated();
	RendererContext* _context;
	std::thread _rendering_thread;
	Array<ResourceHandle> _free_handles;
	RenderResourceHandle _resource_lut[num_handles];
	Vector2u _resolution;
	ResourceHandle _sprite_rendering_quad_handle;

private:
	bool _is_setup;
	bool _active;
	Allocator& _allocator;
	void render_world(const View& view);
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	RenderInterface _render_interface;
};

}