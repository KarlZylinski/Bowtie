#pragma once

#include <mutex>
#include "renderer_command.h"
#include "render_resource_types.h"
#include <foundation/collection_types.h>

namespace bowtie
{

struct Allocator;
struct World;
struct Texture;
struct RenderFence;
struct ConcurrentRingBuffer;

struct RenderInterface
{
	Allocator* allocator;
	ConcurrentRingBuffer* _unprocessed_commands;
	bool* _unprocessed_commands_exist;
	std::mutex* _unprocessed_commands_exist_mutex;
	std::condition_variable* _wait_for_unprocessed_commands_to_exist;
	RenderResourceHandle _free_handles[render_resource_handle::num];
	unsigned num_free_handles;
};

namespace render_interface
{
	void init(RenderInterface* ri, Allocator* allocator, ConcurrentRingBuffer* unprocessed_commands, bool* unprocessed_commands_exist,
			  std::mutex* unprocessed_commands_exist_mutex, std::condition_variable* wait_for_unprocessed_commands_to_exist);
	RenderResourceHandle create_handle(RenderInterface* ri);
	void free_handle(RenderInterface* ri, RenderResourceHandle handle);
	void create_texture(RenderInterface* ri, Texture* texture);
	void create_render_world(RenderInterface* ri, World* world);
	RendererCommand create_command(RenderInterface* ri, RendererCommand::Type type);
	void dispatch(RenderInterface* ri, RendererCommand* command, void* dynamic_data, unsigned dynamic_data_size);
	void dispatch(RenderInterface* ri, RendererCommand* command);
	void create_resource(RenderInterface* ri, RenderResourceData* resource, void* dynamic_data, unsigned dynamic_data_size);
	void update_resource(RenderInterface* ri, RenderResourceData* resource, void* dynamic_data, unsigned dynamic_data_size);
	void create_resource(RenderInterface* ri, RenderResourceData* resource);
	void update_resource(RenderInterface* ri, RenderResourceData* resource);
	RenderFence* create_fence(RenderInterface* ri);
	void wait_for_fence(RenderInterface* ri, RenderFence* fence);
	void wait_until_idle(RenderInterface* ri);
	void resize(RenderInterface* ri, const Vector2u* resolution);
}

}