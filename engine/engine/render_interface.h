#pragma once

#include <mutex>
#include "renderer_command.h"
#include "render_resource_types.h"
#include <foundation/collection_types.h>

namespace bowtie
{

class Allocator;
struct World;
struct Texture;
struct RenderFence;
class ResourceManager;

struct RenderInterface
{
	Allocator* allocator;
	Array<RendererCommand>* _unprocessed_commands;
	std::mutex* _unprocessed_commands_mutex;
	bool* _unprocessed_commands_exist;
	std::mutex* _unprocessed_commands_exist_mutex;
	std::condition_variable* _wait_for_unprocessed_commands_to_exist;
	Array<RenderResourceHandle> _free_handles;
};

namespace render_interface
{
	void init(RenderInterface& ri, Allocator& allocator, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex,
		bool& unprocessed_commands_exist, std::mutex& unprocessed_commands_exist_mutex, std::condition_variable& wait_for_unprocessed_commands_to_exist);
	void deinit(RenderInterface& ri);
	RenderResourceHandle create_handle(RenderInterface& ri);
	void free_handle(RenderInterface& ri, RenderResourceHandle handle);
	void create_texture(RenderInterface& ri, Texture& texture);
	void create_render_world(RenderInterface& ri, World& world);
	RendererCommand create_command(RenderInterface& ri, RendererCommand::Type type);
	void dispatch(RenderInterface& ri, const RendererCommand& command, void* dynamic_data, unsigned dynamic_data_size);
	void dispatch(RenderInterface& ri, const RendererCommand& command);
	void create_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size);
	void update_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size);
	void create_resource(RenderInterface& ri, RenderResourceData& resource);
	void update_resource(RenderInterface& ri, RenderResourceData& resource);
	RenderFence& create_fence(RenderInterface& ri);
	void wait_for_fence(RenderInterface& ri, RenderFence& fence);
	void wait_until_idle(RenderInterface& ri);
	void resize(RenderInterface& ri, const Vector2u& resolution);
}

}