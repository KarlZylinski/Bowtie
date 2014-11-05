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

namespace rectangle_renderer_component_script_interface
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
		auto& transform = world.transform_component();
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

	auto& component = world.rectangle_renderer_component();
	rectangle_renderer_component::create(component, entity, *s_allocator, rect, color);
	world.handle_rectangle_renderer_created(entity);
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int destroy(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	rectangle_renderer_component::destroy(*(RectangleRendererComponent*)c.component, c.entity);
	return 0;
}

int get(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	auto& component = world.rectangle_renderer_component();
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int rect(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	const auto& rect = rectangle_renderer_component::rect(*(RectangleRendererComponent*)c.component, c.entity);
	script_interface::push_vector2(lua, rect.position);
	script_interface::push_vector2(lua, rect.size);
	return 2;
}

int set_rect(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	Rect rect(script_interface::to_vector2(lua, 2), script_interface::to_vector2(lua, 3));
	rectangle_renderer_component::set_rect(*(RectangleRendererComponent*)c.component, c.entity, rect);
	return 0;
}

int color(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	const auto& color = rectangle_renderer_component::color(*(RectangleRendererComponent*)c.component, c.entity);
	script_interface::push_color(lua, color);
	return 1;
}

int set_color(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	auto color = script_interface::to_color(lua, 2);
	rectangle_renderer_component::set_color(*(RectangleRendererComponent*)c.component, c.entity, color);
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
		{ "set_color", set_color }
	};

	script_interface::register_interface(lua, "RectangleRendererComponent", functions, 7);
}

} // namespace drawable_script_interface
} // namespace bowtie