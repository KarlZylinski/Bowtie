#include "script_sprite.h"

#include <lua.hpp>

#include <engine/world.h>

namespace bowtie
{
namespace sprite_script_interface
{

int create_sprite(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 0);
	auto name = lua_tostring(lua, 1);

	world.

	return 0;
}

void load(lua_State* lua)
{
	lua_pushcfunction(lua, create_sprite);
    lua_setglobal(lua, "create_sprite");
}

} // namespace sprite_script_interface
} // namespace bowtie