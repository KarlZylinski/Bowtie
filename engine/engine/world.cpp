#include "world.h"

#include <cassert>

#include <foundation/array.h>

#include "render_interface.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface) : _sprites(allocator), _render_interface(render_interface)
{
}

void World::set_render_handle(ResourceHandle render_handle)
{
	assert(_render_handle.type != ResourceHandle::NotInitialized && "_render_handle already initialized.");

	_render_handle = render_handle;
}

void World::add_sprite(Sprite* sprite)
{
	assert(sprite != nullptr);

	array::push_back(_sprites, sprite);
	_render_interface.create_sprite(/* FUCKER WE NEED TEXTURE HERE*/, *this);
}

void World::remove_sprite(Sprite* )
{

}

ResourceHandle World::render_handle()
{
	return _render_handle;
}

const Array<Sprite*>& World::sprites() const
{
	return _sprites;
}

} // namespace bowtie