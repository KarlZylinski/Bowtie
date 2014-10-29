#include "script_entity.h"

#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include "script_interface_helpers.h"
#include "script_console.h"

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
	rectangle_renderer_component::create(world.rectangle_renderer_component(), entity, *s_allocator);
	return 0;
}

int rect(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	const Rect& rect = rectangle_renderer_component::rect(world.rectangle_renderer_component(), entity);
	script_interface::push_vector2(lua, rect.position);
	script_interface::push_vector2(lua, rect.size);
	return 2;
}

int set_rect(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	Rect rect(script_interface::to_vector2(lua, 3), script_interface::to_vector2(lua, 4));
	rectangle_renderer_component::set_rect(world.rectangle_renderer_component(), entity, rect);
	return 0;
}

void load(lua_State* lua, Allocator& allocator)
{
	s_allocator = &allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "rect", rect },
		{ "set_rect", set_rect }
	};

	script_interface::register_interface(lua, "RectangleRendererComponent", functions, 3);
}

} // namespace drawable_script_interface
} // namespace bowtie