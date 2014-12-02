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

namespace transform_component_script_interface
{

namespace
{
	Allocator* s_allocator = nullptr;
}

int create(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	auto& component = world.transform_components();
	transform_component::create(component, entity, *s_allocator);
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int destroy(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	transform_component::destroy(*(TransformComponent*)c.component, c.entity);
	return 0;
}

int get(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	World& world = *(World*)lua_touserdata(lua, 2);
	auto& component = world.transform_components();
	script_interface::push_component(lua, &component, entity);
	return 1;
}

int position(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	script_interface::push_vector2(lua, transform_component::position(*(TransformComponent*)c.component, c.entity));
	return 1;
}

int set_position(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	transform_component::set_position(*(TransformComponent*)c.component, c.entity, script_interface::to_vector2(lua, 2));
	return 0;
}

int rotation(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	lua_pushnumber(lua, transform_component::rotation(*(TransformComponent*)c.component, c.entity));
	return 1;
}

int set_rotation(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	transform_component::set_rotation(*(TransformComponent*)c.component, c.entity, (float)lua_tonumber(lua, 2));
	return 0;
}

int pivot(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	script_interface::push_vector2(lua, transform_component::pivot(*(TransformComponent*)c.component, c.entity));
	return 1;
}

int set_pivot(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	transform_component::set_pivot(*(TransformComponent*)c.component, c.entity, script_interface::to_vector2(lua, 2));
	return 0;
}

int parent(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	lua_pushnumber(lua, transform_component::parent(*(TransformComponent*)c.component, c.entity));
	return 1;
}

int set_parent(lua_State* lua)
{
	auto c = script_interface::to_component(lua, 1);
	transform_component::set_parent(*(TransformComponent*)c.component, c.entity, (unsigned)lua_tonumber(lua, 2));
	return 0;
}

void load(lua_State* lua, Allocator& allocator)
{
	s_allocator = &allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "get", get },
		{ "position", position },
		{ "set_position", set_position },
		{ "rotation", rotation },
		{ "set_rotation", set_rotation },
		{ "pivot", pivot },
		{ "set_pivot", set_pivot },
		{ "parent", parent },
		{ "set_parent", set_parent }
	};

	script_interface::register_interface(lua, "Transform", functions, 11);
}

} // namespace drawable_script_interface
} // namespace bowtie