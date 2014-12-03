#include "world.h"

#include <cassert>
#include <foundation/array.h>
#include <foundation/quad.h>
#include "material.h"
#include "render_interface.h"
#include "resource_manager.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_allocator(allocator), _render_interface(render_interface), _resource_manager(resource_manager)
{
	sprite_renderer_component::init(_sprite_renderer_components, allocator);
	transform_component::init(_transform_components, allocator);
}

World::~World()
{
	sprite_renderer_component::deinit(_sprite_renderer_components, _allocator);
	transform_component::deinit(_transform_components, _allocator);
}

void World::set_render_handle(RenderResourceHandle render_handle)
{
	_render_handle = render_handle;
}

RenderResourceHandle World::render_handle()
{
	return _render_handle;
}

SpriteRendererComponent& World::sprite_renderer_components()
{
	return _sprite_renderer_components;
}

TransformComponent& World::transform_components()
{
	return _transform_components;
}

Matrix4 world_matrix(const TransformComponentData& c, unsigned i)
{
	auto parent_index = c.parent[i];

	auto p = Matrix4();
	p[3][0] = -c.pivot[i].x;
	p[3][1] = -c.pivot[i].y;

	if (parent_index != transform_component::not_assigned)
	{
		p[3][0] += c.pivot[parent_index].x;
		p[3][1] += c.pivot[parent_index].y;
	}

	auto t = Matrix4();
	t[0][0] = cos(c.rotation[i]);
	t[1][0] = -sin(c.rotation[i]);
	t[0][1] = sin(c.rotation[i]);
	t[1][1] = cos(c.rotation[i]);
	t[3][0] = c.position[i].x;
	t[3][1] = c.position[i].y;


	if (parent_index == transform_component::not_assigned)
		return p * t;
	else
		return p * t * c.world_transform[parent_index];
}

void update_transforms(TransformComponentData& transform, unsigned start, unsigned end, SpriteRendererComponent& sprite_renderer)
{
	for (unsigned i = start; i < end; ++i)
	{
		auto entity = transform.entity[i];
		auto world_transform = world_matrix(transform, i);
		transform.world_transform[i] = world_transform;

		if (!component::has_entity(sprite_renderer.header, entity))
			continue;

		auto sprite_index = hash::get(sprite_renderer.header.map, transform.entity[i]);
		auto rect = sprite_renderer.data.rect[sprite_index];
		auto v1 = world_transform * Vector4(rect.position.x, rect.position.y, 0, 1);
		auto v2 = world_transform * Vector4(rect.position.x + rect.size.x, rect.position.y, 0, 1);
		auto v3 = world_transform * Vector4(rect.position.x, rect.position.y + rect.size.y, 0, 1);
		auto v4 = world_transform * Vector4(rect.position.x + rect.size.x, rect.position.y + rect.size.y, 0, 1);

		Quad geometry = {
			Vector2(v1.x, v1.y),
			Vector2(v2.x, v2.y),
			Vector2(v3.x, v3.y),
			Vector2(v4.x, v4.y)
		};

		sprite_renderer_component::set_geometry(sprite_renderer, entity, geometry);
	}
}

void create_sprites(Allocator& allocator, RenderInterface& ri, RenderResourceHandle default_material, RenderResourceHandle render_world, SpriteRendererComponent& sprite_renderer, unsigned num)
{
	auto rrd = ri.create_render_resource_data(RenderResourceData::SpriteRenderer);

	for (unsigned i = sprite_renderer.header.first_new; i < sprite_renderer.header.num; ++i)
	{
		sprite_renderer.data.render_handle[i] = ri.create_handle();

		if (sprite_renderer.data.material[i].render_handle == (unsigned)-1)
			sprite_renderer.data.material[i].render_handle = default_material;
	}

	CreateSpriteRendererData data;
	data.num = num;
	data.world = render_world;
	rrd.data = &data;
	ri.create_resource(rrd, sprite_renderer_component::copy_new_data(sprite_renderer, allocator), sprite_renderer_component::component_size * data.num);
}

void update_sprites(Allocator& allocator, RenderInterface& ri, SpriteRendererComponent& sprite_renderer, unsigned num)
{
	auto rrd = ri.create_render_resource_data(RenderResourceData::SpriteRenderer);
	UpdateSpriteRendererData data;
	data.num = num;
	rrd.data = &data;
	ri.update_resource(rrd, sprite_renderer_component::copy_dirty_data(sprite_renderer, allocator), sprite_renderer_component::component_size * data.num);
}

void World::update()
{
	{
		if (component::num_new(_transform_components.header) > 0)
			update_transforms(_transform_components.data, _transform_components.header.first_new, _transform_components.header.num, _sprite_renderer_components);
	
		component::reset_new(_transform_components.header);	
	}

	{
		const auto num_dirty_transforms = component::num_dirty(_transform_components.header);

		if (num_dirty_transforms > 0)
			update_transforms(_transform_components.data, 0, num_dirty_transforms, _sprite_renderer_components);

		component::reset_dirty(_transform_components.header);
	}
	
	{
		const auto num_new_sprites = component::num_new(_sprite_renderer_components.header);

		if (num_new_sprites > 0)
		{
			auto default_material = ((Material*)_resource_manager.load(ResourceType::Material, "shared/default_resources/rect.material").object)->render_handle;
			create_sprites(_allocator, _render_interface, default_material, _render_handle, _sprite_renderer_components, num_new_sprites);
		}

		component::reset_new(_sprite_renderer_components.header);
	}

	{
		const auto num_dirty_sprites = component::num_dirty(_sprite_renderer_components.header);

		if (num_dirty_sprites > 0)
			update_sprites(_allocator, _render_interface, _sprite_renderer_components, num_dirty_sprites);
	
		component::reset_dirty(_sprite_renderer_components.header);
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

} // namespace bowtie
