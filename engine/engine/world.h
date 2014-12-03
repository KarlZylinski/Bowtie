#pragma once

#include <foundation/collection_types.h>
#include "render_resource_handle.h"
#include "entity/components/sprite_renderer_component.h"
#include "entity/components/transform_component.h"

namespace bowtie
{

typedef unsigned Entity;

struct Vector4;
struct Font;
struct Rect;
class RenderInterface;
class ResourceManager;
class Sprite;
class Text;
class World
{
public:
	World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager);
	~World();

	void set_render_handle(RenderResourceHandle render_handle);
	void update();
	void draw(const Rect& view);
	RenderResourceHandle render_handle();
	SpriteRendererComponent& sprite_renderer_components();
	TransformComponent& transform_components();
private:
	Allocator& _allocator;
	RenderResourceHandle _render_handle;
	RenderInterface& _render_interface;
	ResourceManager& _resource_manager;
	TransformComponent _transform_components;
	SpriteRendererComponent _sprite_renderer_components;

	World(const World&);
	World& operator=(const World&);
};

};
