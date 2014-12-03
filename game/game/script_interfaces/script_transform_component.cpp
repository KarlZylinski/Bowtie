#include "script_entity.h"
#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include <foundation/vector4.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/rect.h>
#include <cassert>

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
	auto e = script_interface::to_entity(lua, 1);
	transform_component::create(e.world->transform_components(), e.entity, *s_allocator);
	return 0;
}

int destroy(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	transform_component::destroy(e.world->transform_components(), e.entity);
	return 0;
}

int position(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	script_interface::push_vector2(lua, transform_component::position(e.world->transform_components(), e.entity));
	return 1;
}

int set_position(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	transform_component::set_position(e.world->transform_components(), e.entity, script_interface::to_vector2(lua, 2));
	return 0;
}

int rotation(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	lua_pushnumber(lua, transform_component::rotation(e.world->transform_components(), e.entity));
	return 1;
}

int set_rotation(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	transform_component::set_rotation(e.world->transform_components(), e.entity, (float)lua_tonumber(lua, 2));
	return 0;
}

int pivot(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	script_interface::push_vector2(lua, transform_component::pivot(e.world->transform_components(), e.entity));
	return 1;
}

int set_pivot(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	transform_component::set_pivot(e.world->transform_components(), e.entity, script_interface::to_vector2(lua, 2));
	return 0;
}

int parent(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto parent = transform_component::parent(e.world->transform_components(), e.entity);
	script_interface::push_entity(lua, parent, e.world);
	return 1;
}

int set_parent(lua_State* lua)
{
	auto e = script_interface::to_entity(lua, 1);
	auto parent = script_interface::to_entity(lua, 2);
	assert(e.world == parent.world);
	transform_component::set_parent(e.world->transform_components(), e.entity, parent.entity);
	return 0;
}

void load(lua_State* lua, Allocator& allocator)
{
	s_allocator = &allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "position", position },
		{ "set_position", set_position },
		{ "rotation", rotation },
		{ "set_rotation", set_rotation },
		{ "pivot", pivot },
		{ "set_pivot", set_pivot },
		{ "parent", parent },
		{ "set_parent", set_parent }
	};

	script_interface::register_interface(lua, "Transform", functions, 10);
}

} // namespace transform_component_script_interface
} // namespace bowtie
