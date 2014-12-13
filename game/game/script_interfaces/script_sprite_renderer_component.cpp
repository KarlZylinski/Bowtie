#include "script_entity.h"
#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include <foundation/vector4.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/rect.h>
#include <engine/world.h>

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
	auto e = script_interface::to_entity(lua, 1);
	
	{
		auto& transform = e.world->transform_components;
		if (!component::has_entity(&transform.header, e.entity))
			transform_component::create(&transform, e.entity, s_allocator);
	}

	Vector2 position;

	if (script_interface::is_vector2(lua, 2))
		position = script_interface::to_vector2(lua, 2);

	Vector2 size;

	if (script_interface::is_vector2(lua, 3))
		size = script_interface::to_vector2(lua, 3);

	Color color = {1, 1, 1, 1};

	if (script_interface::is_color(lua, 4))
		color = script_interface::to_color(lua, 4);

	Rect rect;
	rect::init(&rect, &position, &size);
	sprite_renderer_component::create(&e.world->sprite_renderer_components, e.entity, s_allocator, &rect, &color);
	return 0;
}

int destroy(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	sprite_renderer_component::destroy(&e.world->sprite_renderer_components, e.entity);
	return 0;
}

int rect(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto rect = sprite_renderer_component::rect(&e.world->sprite_renderer_components, e.entity);
	script_interface::push_vector2(lua, &rect->position);
	script_interface::push_vector2(lua, &rect->size);
	return 2;
}

int set_rect(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto position = script_interface::to_vector2(lua, 2);
	auto size = script_interface::to_vector2(lua, 3);
	Rect rect;
	rect::init(&rect, &position, &size);
	sprite_renderer_component::set_rect(&e.world->sprite_renderer_components, e.entity, &rect);
	return 0;
}

int color(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto color = sprite_renderer_component::color(&e.world->sprite_renderer_components, e.entity);
	script_interface::push_color(lua, color);
	return 1;
}

int set_color(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto color = script_interface::to_color(lua, 2);
	sprite_renderer_component::set_color(&e.world->sprite_renderer_components, e.entity, &color);
	return 0;
}

int set_material(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto material = (Material*)lua_touserdata(lua, 2);
	sprite_renderer_component::set_material(&e.world->sprite_renderer_components, e.entity, material);
	return 0;
}

int set_depth(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	sprite_renderer_component::set_depth(&e.world->sprite_renderer_components, e.entity, (int)lua_tonumber(lua, 2));
	return 0;
}

void load(lua_State* lua, Allocator* allocator)
{
	s_allocator = allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "rect", rect },
		{ "set_rect", set_rect },
		{ "color", color },
		{ "set_color", set_color },
		{ "set_material", set_material },
		{ "set_depth", set_depth }
	};

	script_interface::register_interface(lua, "SpriteRenderer", functions, 8);
}

} // namespace sprite_renderer_component_script_interface
} // namespace bowtie
