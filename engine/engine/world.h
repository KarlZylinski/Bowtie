#pragma once

#include <foundation/collection_types.h>
#include "resource_handle.h"

namespace bowtie
{

class Sprite;
class Drawable;
class RenderInterface;
class ResourceManager;
class Text;
class Font;
class View;
class World
{
public:
	World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager);
	~World();

	void set_render_handle(ResourceHandle render_handle);
	Sprite* spawn_sprite(const char* name);
	void despawn_sprite(Sprite* sprite);
	Text* spawn_text(const Font& font, const char* text);
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
