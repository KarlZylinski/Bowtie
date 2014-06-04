#pragma once

#include <foundation/collection_types.h>
#include "resource_handle.h"

namespace bowtie
{

class Sprite;
class RenderInterface;
class ResourceManager;
class World
{
public:
	World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager);
	~World();

	void set_render_handle(ResourceHandle render_handle);
	Sprite* spawn_sprite(const char* name);
	void despawn_sprite(Sprite* sprite);
	void update();
	void draw();
	ResourceHandle render_handle();

	const Array<Sprite*>& sprites() const;
private:
	Allocator& _allocator;
	Array<Sprite*> _sprites;
	ResourceHandle _render_handle;
	RenderInterface& _render_interface;
	ResourceManager& _resource_manager;

	World(const World&);
	World& operator=(const World&);
};

};
