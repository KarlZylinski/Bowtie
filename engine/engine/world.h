#pragma once

#include <foundation/collection_types.h>
#include "render_resource_handle.h"
#include "entity/components/rectangle_renderer_component.h"

namespace bowtie
{

typedef unsigned Entity;

struct Vector4;
class Drawable;
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
	void add_rectangle_component(Entity e);
	Drawable* spawn_rectangle(const Rect& rect, const Vector4& color, int depth);
	Drawable* spawn_sprite(const char* name, int depth);
	void unspawn(Drawable& sprite);
	Drawable* spawn_text(const Font& font, const char* text, int depth);
	void update();
	void draw(const Rect& view);
	RenderResourceHandle render_handle();

	const Array<Drawable*>& drawables() const;
private:
	Allocator& _allocator;
	Array<Drawable*> _drawables;
	RenderResourceHandle _render_handle;
	RenderInterface& _render_interface;
	ResourceManager& _resource_manager;
	RectangleRendererComponent _rectangle_renderer_component;

	World(const World&);
	World& operator=(const World&);
};

};
