#pragma once

#include <foundation/collection_types.h>
#include "resource_handle.h"

namespace bowtie
{

class Sprite;
class RenderInterface;
class World
{
public:
	World(Allocator& allocator, RenderInterface& render_interface);

	void set_render_handle(ResourceHandle render_handle);
	void add_sprite(Sprite* sprite);
	void remove_sprite(Sprite* sprite);
	ResourceHandle render_handle();

	const Array<Sprite*>& sprites() const;
private:
	Array<Sprite*> _sprites;
	ResourceHandle _render_handle;
	RenderInterface& _render_interface;
};

};