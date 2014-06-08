#pragma once

#include "renderer_command.h"
#include "render_resource_types.h"
namespace bowtie
{

class Allocator;
class Renderer;
class World;
class Drawable;
struct Texture;
struct RenderFence;
class ResourceManager;
class RenderInterface
{
public:
	RenderInterface(Renderer& renderer, Allocator& allocator);
	
	bool is_setup() const;
	bool active() const;
	
	void create_texture(Texture& texture);
	void spawn(World& world, Drawable& drawable, ResourceManager& resource_manager);
	void create_render_world(World& world);
	RenderResourceData create_render_resource_data(RenderResourceData::Type type);
	RendererCommand create_command(RendererCommand::Type type);
	void dispatch(const RendererCommand& command);
	void create_resource(RenderResourceData& resource, void* dynamic_data = nullptr, unsigned dynamic_data_size = 0);

	RenderFence& create_fence();
	void wait_for_fence(RenderFence& fence);
	void resize(const Vector2u& resolution);
	const Vector2u& resolution() const;

private:
	Allocator& _allocator;
	Renderer& _renderer;

	RenderInterface(const RenderInterface&);
	RenderInterface& operator=(const RenderInterface&);
};

}