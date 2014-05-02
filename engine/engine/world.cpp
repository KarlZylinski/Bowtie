#include "world.h"

#include <cassert>

#include <foundation/array.h>

#include "render_interface.h"
#include "resource_manager.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_sprites(allocator), _render_interface(render_interface), _resource_manager(resource_manager)
{
}

void World::set_render_handle(ResourceHandle render_handle)
{
	assert(_render_handle.type != ResourceHandle::NotInitialized && "_render_handle already initialized.");

	_render_handle = render_handle;
}

void World::spawn_sprite(uint64_t sprite_name)
{
	auto sprite = _resource_manager.get<Sprite>(ResourceManager::RT_Sprite, sprite_name);

	array::push_back(_sprites, sprite);
	_render_interface.spawn_sprite(*this, *sprite);
}

void World::despawn_sprite(Sprite* )
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