#pragma once

#include "renderer_command.h"
#include "render_resource_types.h"

namespace bowtie
{

class Allocator;
class IRenderer;
struct World;
struct Texture;
struct RenderFence;
class ResourceManager;

struct RenderInterface
{
	Allocator* allocator;
	IRenderer* renderer;
};

namespace render_interface
{
	void init(RenderInterface& ri, Allocator& allocator, IRenderer& renderer);
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