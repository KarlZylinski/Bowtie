#include "world.h"

#include <cassert>

#include <foundation/array.h>

#include "render_interface.h"
#include "resource_manager.h"
#include "sprite.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_allocator(allocator), _sprites(allocator), _render_interface(render_interface), _resource_manager(resource_manager)
{
}

World::~World()
{
	for (unsigned i = 0; i < array::size(_sprites); ++i)
		_allocator.deallocate(_sprites[i]);
}

void World::set_render_handle(ResourceHandle render_handle)
{
	assert(_render_handle.type == ResourceHandle::NotInitialized && "_render_handle already initialized.");

	_render_handle = render_handle;
}

Sprite* World::spawn_sprite(const char* sprite_name)
{
	_resource_manager.load_sprite_prototype(sprite_name);
	auto sprite_prototype = _resource_manager.get<Sprite>(ResourceManager::RT_Sprite, sprite_name);

	auto sprite = (Sprite*)_allocator.allocate(sizeof(Sprite));
	memcpy(sprite, sprite_prototype, sizeof(Sprite));

	array::push_back(_sprites, sprite);
	_render_interface.spawn_sprite(*this, *sprite);

	return sprite;
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

void World::update()
{
	for (unsigned i = 0; i < array::size(_sprites); ++i)
	{
		auto sprite = _sprites[i];
		
		if (!sprite->state_changed())
			continue;

		auto state_changed_command = _render_interface.create_command(RendererCommand::SpriteStateReflection);
		
		auto& scd = *(SpriteStateReflectionData*)_allocator.allocate(sizeof(SpriteStateReflectionData));
		scd.model = sprite->model_matrix();
		scd.sprite = sprite->render_handle();
		state_changed_command.data = &scd;

		sprite->reset_state_changed();

		_render_interface.dispatch(state_changed_command);
	}
}

void World::draw()
{
	auto render_world_command = _render_interface.create_command(RendererCommand::RenderWorld);
	
	View view(Vector2(640,480), Vector2(0,0));

	auto& rwd = *(RenderWorldData*)_allocator.allocate(sizeof(RenderWorldData));
	rwd.view = view;
	rwd.render_world = _render_handle;
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

} // namespace bowtie