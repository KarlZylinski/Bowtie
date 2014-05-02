#include "script_sprite.h"

#include <lua.hpp>


namespace bowtie
{
namespace sprite_script_interface
{

void load(lua_State* lua)
{
    lua_setglobal(lua, "create_sprite");
}

} // namespace sprite_script_interface
} // namespace bowtie