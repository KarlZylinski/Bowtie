#include "world.h"

#include <cassert>

#include <foundation/array.h>

#include "render_interface.h"
#include "resource_manager.h"
#include "sprite.h"
#include "text.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_allocator(allocator), _drawables(allocator), _render_interface(render_interface), _resource_manager(resource_manager)
{
}

World::~World()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
		MAKE_DELETE(_allocator, Drawable, _drawables[i]);
}

void World::set_render_handle(ResourceHandle render_handle)
{
	assert(_render_handle.type == ResourceHandle::NotInitialized && "_render_handle already initialized.");

	_render_handle = render_handle;
}

Sprite* World::spawn_sprite(const char* sprite_name)
{
	auto sprite_prototype = _resource_manager.get<Sprite>(ResourceManager::RT_Sprite, sprite_name);

	auto sprite = MAKE_NEW(_allocator, Sprite, *sprite_prototype);

	array::push_back(_drawables, (Drawable*)sprite);
	_render_interface.spawn(*this, *sprite, _resource_manager);

	return sprite;
}

void World::despawn_sprite(Sprite* )
{

}

ResourceHandle World::render_handle()
{
	return _render_handle;
}

const Array<Drawable*>& World::drawables() const
{
	return _drawables;
}

void update_drawable_state(Allocator& allocator, RenderInterface& render_interface, Drawable& drawable)
{
	auto state_changed_command = render_interface.create_command(RendererCommand::DrawableStateReflection);
		
	auto& scd = *(DrawableStateReflectionData*)allocator.allocate(sizeof(DrawableStateReflectionData));
	scd.model = drawable.model_matrix();
	scd.drawble = drawable.render_handle();
	state_changed_command.data = &scd;

	drawable.reset_state_changed();

	render_interface.dispatch(state_changed_command);
}

void update_drawable_geometry(Allocator& allocator, RenderInterface& render_interface, Drawable& drawable)
{
	auto geometry_changed_command = render_interface.create_command(RendererCommand::DrawableGeometryReflection);
		
	auto& sgr = *(DrawableGeometryReflectionData*)allocator.allocate(sizeof(DrawableGeometryReflectionData));
	sgr.drawable = drawable.render_handle();
	sgr.size = drawable.geometry_size();
	
	geometry_changed_command.data = &sgr;
	geometry_changed_command.dynamic_data_size = sgr.size;
	geometry_changed_command.dynamic_data = allocator.allocate(sgr.size);
	memcpy(geometry_changed_command.dynamic_data, drawable.geometry_data(), sgr.size);

	drawable.reset_geometry_changed();

	render_interface.dispatch(geometry_changed_command);
}

void World::update()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
	{
		auto drawable = _drawables[i];
		
		if (drawable->state_changed())
			update_drawable_state(_allocator, _render_interface, *drawable);
		
		if (drawable->geometry_changed())
			update_drawable_geometry(_allocator, _render_interface, *drawable);
	}
}

void World::draw()
{
	auto render_world_command = _render_interface.create_command(RendererCommand::RenderWorld);
	
	View view(Vector2(640,480), Vector2(-200,-200));

	auto& rwd = *(RenderWorldData*)_allocator.allocate(sizeof(RenderWorldData));
	rwd.view = view;
	rwd.render_world = _render_handle;
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

Text* World::spawn_text(const Font& font, const char* text_str)
{
	auto text = MAKE_NEW(_allocator, Text, font, _allocator);
	text->set_text(text_str);
	array::push_back(_drawables, (Drawable*)text);
	_render_interface.spawn(*this, *text, _resource_manager);
	return text;
}

} // namespace bowtie