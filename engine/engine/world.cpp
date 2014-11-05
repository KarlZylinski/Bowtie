#include "world.h"

#include <cassert>
#include <foundation/array.h>
#include <foundation/quad.h>
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
	rectangle_renderer_component::init(_rectangle_renderer_component, allocator);
	transform_component::init(_transform_component, allocator);
}

World::~World()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
		_allocator.destroy(_drawables[i]);

	array::deinit(_drawables);
	rectangle_renderer_component::deinit(_rectangle_renderer_component, _allocator);
	transform_component::deinit(_transform_component, _allocator);
}

void World::set_render_handle(RenderResourceHandle render_handle)
{
	_render_handle = render_handle;
}

Drawable* World::spawn_rectangle(const Rect& rect, const Color& color, int depth)
{
	auto geometry = _allocator.construct<RectangleGeometry>(color, rect);
	auto material = _resource_manager.load(ResourceType::Material, "shared/default_resources/rect.material");
	auto rectangle = _allocator.construct<Drawable>(_allocator, *geometry, (Material*)material.object, depth);
	rectangle->set_position(rect.position);
	array::push_back(_drawables, rectangle);
	_render_interface.spawn(*this, *rectangle, _resource_manager);
	return rectangle;
}

Drawable* World::spawn_sprite(const char* sprite_name, int depth)
{
	auto sprite_prototype = _resource_manager.get<Drawable>(ResourceType::Drawable, sprite_name);
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

void World::spawn_rectangle_component_renderer(Entity entity)
{
	auto rrd = _render_interface.create_render_resource_data(RenderResourceData::RectangleRenderer);
	rectangle_renderer_component::set_render_handle(_rectangle_renderer_component, entity, _render_interface.create_handle());
	auto material = ((Material*)_resource_manager.load(ResourceType::Material, "shared/default_resources/rect.material").object)->render_handle;
	rectangle_renderer_component::set_material(_rectangle_renderer_component, entity, material);
	CreateRectangleRendererData data;
	data.num = 1;
	data.world = _render_handle;
	rrd.data = &data;
	auto rectangle_data = rectangle_renderer_component::copy_data(_rectangle_renderer_component, entity, _allocator);
	_render_interface.create_resource(rrd, rectangle_data.color, rectangle_renderer_component::component_size);
}

RenderResourceHandle World::render_handle()
{
	return _render_handle;
}

RectangleRendererComponent& World::rectangle_renderer_component()
{
	return _rectangle_renderer_component;
}

TransformComponent& World::transform_component()
{
	return _transform_component;
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

Matrix4 make_transform_matrix(const Vector2& position, float rotation, const Vector2& pivot)
{
	auto p = Matrix4();
	p[3][0] = (float)-pivot.x;
	p[3][1] = (float)-pivot.y;

	/*if (_parent != nullptr)
	{
		p[3][0] += (float)_parent->pivot().x;
		p[3][1] += (float)_parent->pivot().y;
	}*/

	auto r = Matrix4();
	r[0][0] = cos(rotation);
	r[1][0] = -sin(rotation);
	r[0][1] = sin(rotation);
	r[1][1] = cos(rotation);

	auto t = Matrix4();
	t[3][0] = position.x;
	t[3][1] = position.y;

	return p * r * t;
	/*if (_parent == nullptr)
		return p * r * t;
	else
		return p * r * t * _parent->model_matrix();*/
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

	const auto num_dirty_transforms = component::num_dirty(_transform_component.header);

	if (num_dirty_transforms > 0)
	{
		auto& c = _transform_component.data;

		for (unsigned i = 0; i < num_dirty_transforms; ++i)
		{	
			auto rotation = c.rotation[i];
			auto position = c.position[i];
			auto entity = c.entity[i];
			auto rect = rectangle_renderer_component::rect(_rectangle_renderer_component, entity);

			// Handednessmess? Fix negation of pivot. Move pivot (0, 0) to upper left corner.

			Quad geometry;
			geometry.v1 = position + vector2::rotate(rect.position + Vector2(100, -300), rotation);
			geometry.v2 = position + vector2::rotate(rect.position + Vector2(rect.size.x, 0) + Vector2(100, -300), rotation);
			geometry.v3 = position + vector2::rotate(rect.position + Vector2(0, rect.size.y) + Vector2(100, -300), rotation);
			geometry.v4 = position + vector2::rotate(rect.position + rect.size + Vector2(100, -300), rotation);
			rectangle_renderer_component::set_geometry(_rectangle_renderer_component, entity, geometry);
		}

		component::reset_dirty(_transform_component.header);
	}

	const auto num_dirty_rectangles = component::num_dirty(_rectangle_renderer_component.header);

	if (num_dirty_rectangles > 0)
	{
		auto rrd = _render_interface.create_render_resource_data(RenderResourceData::RectangleRenderer);
		UpdateRectangleRendererData data;
		data.num = num_dirty_rectangles;
		rrd.data = &data;
		auto rectangle_data = rectangle_renderer_component::copy_dirty_data(_rectangle_renderer_component, _allocator);
		_render_interface.update_resource(rrd, rectangle_data.color, rectangle_renderer_component::component_size * data.num);
		component::reset_dirty(_rectangle_renderer_component.header);
	}
}

void World::draw(const Rect& view)
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
	auto text = _allocator.construct<Drawable>(_allocator, *text_geometry, (Material*)_resource_manager.get_default(ResourceType::Material).object, depth);
	array::push_back(_drawables, text);
	_render_interface.spawn(*this, *text, _resource_manager);
	return text;
}

} // namespace bowtie