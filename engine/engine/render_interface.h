#pragma once

#include "renderer_command.h"
#include "render_resource_types.h"
#include "texture.h"
#include "sprite.h"

namespace bowtie
{

class Allocator;
class Renderer;
struct Image;
struct RenderFence;
class RenderInterface
{
public:
	RenderInterface(Renderer& renderer, Allocator& allocator);
	
	bool is_setup() const;
	bool active() const;
	
	Texture* create_texture(const Image& image);
	Sprite create_sprite(const Texture& texture, ResourceHandle render_world);
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