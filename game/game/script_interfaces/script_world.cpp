#include "script_world.h"

#include <lua.hpp>

#include <engine/engine.h>

namespace bowtie
{
namespace world_script_interface
{

int create_world(lua_State* lua)
{
	auto& engine = *(Engine*)lua_touserdata(lua, 0);
	auto world = engine.create_world();
	lua_pushlightuserdata(lua, world);
	return 1;
}

void load(lua_State* lua)
{
	lua_pushcfunction(lua, create_world);
    lua_setglobal(lua, "create_world");
}

} // namespace world_script_interface
} // namespace bowtie