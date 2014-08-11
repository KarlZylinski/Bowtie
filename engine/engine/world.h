#pragma once

#include <foundation/collection_types.h>
#include "resource_handle.h"

namespace bowtie
{

struct Vector4;
class Drawable;
class Font;
struct Rect;
class RenderInterface;
class ResourceManager;
class Sprite;
class Text;
class View;
class World
{
public:
	World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager);
	~World();

	void set_render_handle(ResourceHandle render_handle);
	Drawable* spawn_rectangle(const Rect& rect, const Vector4& color);
	Drawable* spawn_sprite(const char* name);
	void unspawn(Drawable& sprite);
	Drawable* spawn_text(const Font& font, const char* text);
	void update();
	void draw(const View& view);
	ResourceHandle render_handle();

	const Array<Drawable*>& drawables() const;
private:
	Allocator& _allocator;
	Array<Drawable*> _drawables;
	ResourceHandle _render_handle;
	RenderInterface& _render_interface;
	ResourceManager& _resource_manager;

	World(const World&);
	World& operator=(const World&);
};

};
