#include "script_world.h"

#include <lua.hpp>

#include <engine/engine.h>
#include <engine/world.h>

namespace bowtie
{
namespace world_script_interface
{

int create_world(lua_State* lua)
{
	auto& engine = *(Engine*)lua_touserdata(lua, 1);
	auto world = engine.create_world();
	lua_pushlightuserdata(lua, world);
	return 1;
}

int destroy_world(lua_State* lua)
{
	auto& engine = *(Engine*)lua_touserdata(lua, 1);
	auto& world = *(World*)lua_touserdata(lua, 2);
	engine.destroy_world(world);
	return 0;
}

int spawn_sprite(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto name = lua_tointeger(lua, 1);
	
	world.spawn_sprite(name);

	return 0;
}

void load(lua_State* lua)
{
	lua_register(lua, "create_world", create_world);
	lua_register(lua, "destroy_world", destroy_world);
}

} // namespace world_script_interface
} // namespace bowtie