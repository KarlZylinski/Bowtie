#include "script_sprite.h"

#include <lua.hpp>

namespace bowtie
{
namespace sprite_script_interface
{

int create(lua_State* )
{
	return 0;
}

void load(lua_State* lua)
{
	lua_pushcfunction(lua, create);
    lua_setglobal(lua, "create_sprite");
}

} // namespace sprite_script_interface
} // namespace bowtie