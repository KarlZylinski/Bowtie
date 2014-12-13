#include "world.h"

#include <cassert>
#include <foundation/array.h>
#include <foundation/quad.h>
#include "material.h"
#include "render_interface.h"
#include "resource_manager.h"
#include "timer.h"

namespace bowtie
{

namespace
{

Matrix4 world_matrix(const TransformComponentData* c, unsigned i)
{
	auto parent_index = c->parent[i];

	auto p = Matrix4();
	p[3][0] = -c->pivot[i].x;
	p[3][1] = -c->pivot[i].y;

	if (parent_index != transform_component::not_assigned)
	{
		p[3][0] += c->pivot[parent_index].x;
		p[3][1] += c->pivot[parent_index].y;
	}

	auto t = Matrix4();
	t[0][0] = cos(c->rotation[i]);
	t[1][0] = -sin(c->rotation[i]);
	t[0][1] = sin(c->rotation[i]);
	t[1][1] = cos(c->rotation[i]);
	t[3][0] = c->position[i].x;
	t[3][1] = c->position[i].y;


	if (parent_index == transform_component::not_assigned)
		return p * t;
	else
		return p * t * c->world_transform[parent_index];
}

void update_transforms(TransformComponentData* transform, unsigned start, unsigned end, SpriteRendererComponent* sprite_renderer)
{
	for (unsigned i = start; i < end; ++i)
	{
		auto entity = transform->entity[i];
		auto world_transform = world_matrix(transform, i);
		transform->world_transform[i] = world_transform;

		if (!component::has_entity(sprite_renderer->header, entity))
			continue;

		auto sprite_index = hash::get(sprite_renderer->header.map, transform->entity[i]);
		auto rect = sprite_renderer->data.rect[sprite_index];
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

		sprite_renderer_component::set_geometry(*sprite_renderer, entity, geometry);
	}
}

void create_sprites(Allocator* allocator, RenderInterface* ri, RenderResourceHandle default_material, RenderResourceHandle render_world, SpriteRendererComponent* sprite_renderer, unsigned num)
{
	auto rrd = render_resource_data::create(RenderResourceData::SpriteRenderer);

	for (unsigned i = sprite_renderer->header.first_new; i < sprite_renderer->header.num; ++i)
	{
		sprite_renderer->data.render_handle[i] = render_interface::create_handle(ri);

		if (sprite_renderer->data.material[i].render_handle == handle_not_initialized)
			sprite_renderer->data.material[i].render_handle = default_material;
	}

	CreateSpriteRendererData data;
	data.num = num;
	data.world = render_world;
	rrd.data = &data;
	render_interface::create_resource(ri, &rrd, sprite_renderer_component::copy_new_data(*sprite_renderer, *allocator), sprite_renderer_component::component_size * data.num);
}

void update_sprites(Allocator* allocator, RenderInterface* ri, SpriteRendererComponent* sprite_renderer, unsigned num)
{
	auto rrd = render_resource_data::create(RenderResourceData::SpriteRenderer);
	UpdateSpriteRendererData data;
	data.num = num;
	rrd.data = &data;
	render_interface::update_resource(ri, &rrd, sprite_renderer_component::copy_dirty_data(*sprite_renderer, *allocator), sprite_renderer_component::component_size * data.num);
}

} // anonymous namespace

namespace world
{

void init(World* w, Allocator* allocator, RenderInterface* render_interface, ResourceManager* resource_manager)
{
	w->allocator = allocator;
	w->render_interface = render_interface;
	w->render_handle = handle_not_initialized;
	auto default_material = resource_manager::load(*resource_manager, ResourceType::Material, "default.material");
	assert(default_material.is_some && "Default material default.material is missing.");
	w->default_material = ((Material*)default_material.value)->render_handle;
	sprite_renderer_component::init(w->sprite_renderer_components, *allocator);
	transform_component::init(w->transform_components, *allocator);
}

void deinit(World* w)
{
	sprite_renderer_component::deinit(w->sprite_renderer_components, *w->allocator);
	transform_component::deinit(w->transform_components, *w->allocator);
}

void update(World* w)
{
	{
		if (component::num_new(w->transform_components.header) > 0)
			update_transforms(&w->transform_components.data, w->transform_components.header.first_new, w->transform_components.header.num, &w->sprite_renderer_components);
	
		component::reset_new(w->transform_components.header);
	}

	{
		const auto num_dirty_transforms = component::num_dirty(w->transform_components.header);

		if (num_dirty_transforms > 0)
			update_transforms(&w->transform_components.data, 0, num_dirty_transforms, &w->sprite_renderer_components);

		component::reset_dirty(w->transform_components.header);
	}
	
	{
		const auto num_new_sprites = component::num_new(w->sprite_renderer_components.header);

		if (num_new_sprites > 0)
			create_sprites(w->allocator, w->render_interface, w->default_material, w->render_handle, &w->sprite_renderer_components, num_new_sprites);

		component::reset_new(w->sprite_renderer_components.header);
	}

	{
		const auto num_dirty_sprites = component::num_dirty(w->sprite_renderer_components.header);

		if (num_dirty_sprites > 0)
			update_sprites(w->allocator, w->render_interface, &w->sprite_renderer_components, num_dirty_sprites);
	
		component::reset_dirty(w->sprite_renderer_components.header);
	}
}

void draw(World* w, const Rect* view, float time)
{
	auto render_world_command = render_interface::create_command(w->render_interface, RendererCommand::RenderWorld);

	auto rwd = (RenderWorldData*)w->allocator->alloc(sizeof(RenderWorldData));
	rwd->view = *view;
	rwd->render_world = w->render_handle;
	rwd->time = time;
	render_world_command.data = rwd;

	render_interface::dispatch(w->render_interface, &render_world_command);
}

} // namespace world

} // namespace bowtie
