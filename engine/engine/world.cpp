#include "world.h"

#include <cassert>
#include <foundation/array.h>
#include "drawable.h"
#include "material.h"
#include "render_interface.h"
#include "resource_manager.h"
#include "rectangle_geometry.h"
#include "sprite_geometry.h"
#include "text_geometry.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_allocator(allocator), _drawables(array::create<Drawable*>(allocator)), _render_interface(render_interface), _resource_manager(resource_manager)
{
}

World::~World()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
		_allocator.destroy(_drawables[i]);

	array::deinit(_drawables);
}

void World::set_render_handle(RenderResourceHandle render_handle)
{
	_render_handle = render_handle;
}

Drawable* World::spawn_rectangle(const Rect& rect, const Color& color, int depth)
{
	auto geometry = _allocator.construct<RectangleGeometry>(color, rect);
	auto material = _resource_manager.load(resource_type::Material, "shared/default_resources/rect.material");
	auto rectangle = _allocator.construct<Drawable>(_allocator, *geometry, (Material*)material.object, depth);
	rectangle->set_position(rect.position);
	array::push_back(_drawables, rectangle);
	_render_interface.spawn(*this, *rectangle, _resource_manager);
	return rectangle;
}

Drawable* World::spawn_sprite(const char* sprite_name, int depth)
{
	auto sprite_prototype = _resource_manager.get<Drawable>(resource_type::Drawable, sprite_name);
	auto sprite = _allocator.construct<Drawable>(*sprite_prototype);
	sprite->set_depth(depth);
	array::push_back(_drawables, sprite);
	_render_interface.spawn(*this, *sprite, _resource_manager);

	return sprite;
}

void World::unspawn(Drawable& drawable)
{
	array::remove(_drawables, &drawable);
	_render_interface.unspawn(*this, drawable);
	_allocator.destroy(&drawable);
}

RenderResourceHandle World::render_handle()
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
	scd.material = drawable.material()->render_handle;
	scd.drawble = drawable.render_handle();
	scd.depth = drawable.depth();
	state_changed_command.data = &scd;

	drawable.reset_state_changed();

	render_interface.dispatch(state_changed_command);
}

void update_drawable_geometry(Allocator& allocator, RenderInterface& render_interface, Drawable& drawable)
{
	auto geometry_changed_command = render_interface.create_command(RendererCommand::DrawableGeometryReflection);
		
	auto& sgr = *(DrawableGeometryReflectionData*)allocator.allocate(sizeof(DrawableGeometryReflectionData));
	sgr.drawable = drawable.render_handle();
	sgr.size = drawable.geometry().data_size();
	
	geometry_changed_command.data = &sgr;
	geometry_changed_command.dynamic_data_size = sgr.size;
	geometry_changed_command.dynamic_data = allocator.allocate(sgr.size);
	memcpy(geometry_changed_command.dynamic_data, drawable.geometry().data(), sgr.size);

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

void World::draw(const View& view)
{
	auto render_world_command = _render_interface.create_command(RendererCommand::RenderWorld);

	auto& rwd = *(RenderWorldData*)_allocator.allocate(sizeof(RenderWorldData));
	rwd.view = view;
	rwd.render_world = _render_handle;
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

Drawable* World::spawn_text(const Font& font, const char* text_str, int depth)
{
	auto text_geometry = _allocator.construct<TextGeometry>(font, _allocator);
	text_geometry->set_text(text_str);
	auto text = _allocator.construct<Drawable>(_allocator, *text_geometry, (Material*)_resource_manager.get_default(resource_type::Material).object, depth);
	array::push_back(_drawables, text);
	_render_interface.spawn(*this, *text, _resource_manager);
	return text;
}

} // namespace bowtie