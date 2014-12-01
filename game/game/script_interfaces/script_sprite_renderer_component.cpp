#include "script_entity.h"
#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include <foundation/vector4.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/rect.h>

namespace bowtie
{

namespace sprite_renderer_component_script_interface
{

namespace
{
	Allocator* s_allocator = nullptr;
}

int create(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	
	{
		auto& transform = world.transform_components();
		if (!component::has_entity(transform.header, entity))
			transform_component::create(transform, entity, *s_allocator);
	}

	Vector2 position;

	if (script_interface::is_vector2(lua, 3))
		position = script_interface::to_vector2(lua, 3);

	Vector2 size;

	if (script_interface::is_vector2(lua, 4))
		size = script_interface::to_vector2(lua, 4);

	Color color(1, 1, 1, 1);

	if (script_interface::is_color(lua, 5))
		color = script_interface::to_color(lua, 5);

	Rect rect(position, size);

	auto& component = world.sprite_renderer_components();
	sprite_renderer_component::create(component, entity, *s_allocator, rect, color);
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int destroy(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	sprite_renderer_component::destroy(*(SpriteRendererComponent*)c.component, c.entity);
	return 0;
}

int get(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	auto& component = world.sprite_renderer_components();
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int rect(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	const auto& rect = sprite_renderer_component::rect(*(SpriteRendererComponent*)c.component, c.entity);
	script_interface::push_vector2(lua, rect.position);
	script_interface::push_vector2(lua, rect.size);
	return 2;
}

int set_rect(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	Rect rect(script_interface::to_vector2(lua, 2), script_interface::to_vector2(lua, 3));
	sprite_renderer_component::set_rect(*(SpriteRendererComponent*)c.component, c.entity, rect);
	return 0;
}

int color(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	const auto& color = sprite_renderer_component::color(*(SpriteRendererComponent*)c.component, c.entity);
	script_interface::push_color(lua, color);
	return 1;
}

int set_color(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	auto color = script_interface::to_color(lua, 2);
	sprite_renderer_component::set_color(*(SpriteRendererComponent*)c.component, c.entity, color);
	return 0;
}

int set_material(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	auto& material = *(Material*)lua_touserdata(lua, 2);
	sprite_renderer_component::set_material(*(SpriteRendererComponent*)c.component, c.entity, material);
	return 0;
}

void load(lua_State* lua, Allocator& allocator)
{
	s_allocator = &allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "get", get },
		{ "rect", rect },
		{ "set_rect", set_rect },
		{ "color", color },
		{ "set_color", set_color },
		{ "set_material", set_material }
	};

	script_interface::register_interface(lua, "SpriteRendererComponent", functions, 8);
}

} // namespace drawable_script_interface
} // namespace bowtie