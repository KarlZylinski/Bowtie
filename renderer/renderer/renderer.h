#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <engine/irenderer.h>
#include <engine/renderer_command.h>
#include <engine/render_interface.h>
#include <engine/render_resource_types.h>
#include <foundation/collection_types.h>

#include "render_resource_lookup_table.h"
#include "render_resource_handle.h"
#include "renderer_context.h"
#include "render_world.h"

namespace bowtie
{

class IConcreteRenderer;
struct RenderTarget;

struct RendererResourceObject
{
	RenderResourceData::Type type;
	RenderResourceHandle handle;
};

class Renderer : public IRenderer
{
public:
	Renderer(IConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, RenderResourceLookupTable& render_resource_lookup_table);
	~Renderer();
	
	bool is_active() const;
	const Vector2u& resolution() const;
	void add_renderer_command(const RendererCommand& command);
	void deallocate_processed_commands(Allocator& allocator);
	void create_resource(RenderResourceData& render_resource, void* dynamic_data);
	void consume_command_queue();
	ResourceHandle create_handle();
	void flip();
	RenderInterface& render_interface() { return _render_interface; }
	void run(RendererContext* context, const Vector2u& resolution);
	bool is_setup() const { return _setup; }
	
private:	
	RenderResourceHandle create_drawable(DrawableResourceData& drawable_data);
	RenderResourceHandle create_world();
	void drawable_state_reflection(const DrawableStateReflectionData& reflection_data);
	RenderResourceHandle lookup_resource(ResourceHandle handle) const;	
	void move_unprocessed_commands();
	void notify_command_queue_populated();
	void thread();
	
	Allocator& _allocator;
	Array<RendererCommand> _command_queue;
	bool _command_queue_populated;
	std::mutex _command_queue_populated_mutex;
	IConcreteRenderer& _concrete_renderer;
	RendererContext* _context;
	Array<ResourceHandle> _free_handles;
	Array<RendererResourceObject> _resource_objects;
	std::thread _thread;
	std::condition_variable _wait_for_command_queue_populated;

	// Unsorted.
	bool _setup;
	bool _active;
	void render_world(const View& view, ResourceHandle render_world);
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	Array<void*> _processed_memory;
	std::mutex _processed_memory_mutex;
	Array<RenderTarget*> _render_targets;
	RenderInterface _render_interface;
	RenderResourceLookupTable& _render_resource_lookup_table;
	Array<ResourceHandle> _rendered_worlds; // filled each frame with all rendered world, in order
};

}
