#include "script_console.h"

#include <lua.hpp>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace console
{

namespace
{
    lua_State* s_lua;
}

void init(lua_State* )
{
    /*s_lua = lua;
    lua_getglobal(lua, "Console");
    int error = lua_pcall(lua, 1, 1, 0);
    script_interface::check_errors(lua, error);
    lua_setglobal(lua, "console");
    script_interface::check_errors(lua, error);*/
}

void write(const char* )
{
    /*lua_getglobal(s_lua, "console");
    lua_getfield(s_lua, -1, "write");
    lua_pushvalue(s_lua, -2);
    lua_pushstring(s_lua, message);
    int error = lua_pcall(s_lua, 2, 0, 0);
    script_interface::check_errors(s_lua, error); */
}

void update(real32 )
{
    /*lua_getglobal(s_lua, "console");
    lua_getfield(s_lua, -1, "update");
    lua_pushvalue(s_lua, -2);
    lua_pushnumber(s_lua, dt);
    int error = lua_pcall(s_lua, 2, 0, 0);
    script_interface::check_errors(s_lua, error); */
}

void draw()
{
    /*lua_getglobal(s_lua, "console");
    lua_getfield(s_lua, -1, "draw");
    lua_pushvalue(s_lua, -2);
    int error = lua_pcall(s_lua, 1, 0, 0);
    script_interface::check_errors(s_lua, error); */
}

} // namespace console
} // namespace bowtie