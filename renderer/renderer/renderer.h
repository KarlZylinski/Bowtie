#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <engine/irenderer.h>
#include <engine/renderer_command.h>
#include <engine/render_interface.h>
#include <engine/render_resource_types.h>
#include <foundation/collection_types.h>

#include "render_resource_handle.h"
#include "renderer_context.h"
#include "render_world.h"

namespace bowtie
{

struct RenderTarget;

struct RendererResourceObject
{
	RenderResourceData::Type type;
	RenderResourceHandle handle;
};

class Renderer : public IRenderer
{
public:
	static const unsigned num_handles = 4000;

	Renderer(Allocator& renderer_allocator, Allocator& render_interface_allocator);
	virtual ~Renderer();
	
	bool is_active() const { return _active; }
	const Vector2u& resolution() const { return _resolution; }
	void add_renderer_command(const RendererCommand& command);
	void deallocate_processed_commands(Allocator& allocator);
	void create_resource(RenderResourceData& render_resource, void* dynamic_data);
	void consume_command_queue();
	ResourceHandle create_handle();
	RenderInterface& render_interface() { return _render_interface; }
	void run(RendererContext* context, const Vector2u& resolution);
	RenderResourceHandle lookup_resource_object(ResourceHandle handle) const;
	bool is_setup() const { return _is_setup; }
	
	// Renderer API specific
	virtual void draw(const View& view, ResourceHandle render_world_handle) = 0;
	virtual void set_render_target(const RenderTarget& render_target) = 0;
	virtual void clear() = 0;
	virtual void flip() = 0;

	virtual void combine_rendered_worlds(const Array<ResourceHandle>& rendered_worlds) = 0;
	virtual void resize(const Vector2u& size, Array<RenderTarget*>& render_targets) = 0;
	virtual RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data) = 0;
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data) = 0;
	virtual RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data) = 0;
	virtual void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data) = 0;
	virtual RenderTarget* create_render_target() = 0;
	
protected:
	virtual void run_thread() = 0;
	void move_unprocessed_commands();
	void set_active(bool active) { _active = active; }
		
	Allocator& _allocator;
	Array<RendererCommand> _command_queue;
	bool _command_queue_populated;
	std::mutex _command_queue_populated_mutex;
	std::condition_variable _wait_for_command_queue_populated;
	void notify_command_queue_populated();
	RendererContext* _context;
	std::thread _rendering_thread;
	Array<ResourceHandle> _free_handles;
	RenderResourceHandle _resource_lut[num_handles];
	Array<RendererResourceObject> _resource_objects;
	Vector2u _resolution;

private:
	void drawable_state_reflection(const DrawableStateReflectionData& reflection_data);
	RenderResourceHandle create_drawable(DrawableResourceData& drawable_data);
	RenderResourceHandle create_world();

	bool _is_setup;
	bool _active;
	void render_world(const View& view, ResourceHandle render_world);
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	Array<RendererCommand> _processed_commands;
	std::mutex _processed_commands_mutex;
	Array<RenderTarget*> _render_targets;
	RenderInterface _render_interface;
	Array<ResourceHandle> _rendered_worlds; // filled each frame with all rendered world, in order
};

}
