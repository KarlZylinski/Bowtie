#include "script_sprite.h"

#include <cmath>

#include <lua.hpp>

#include <engine/sprite.h>

#include "script_interface_helpers.h"

#include <engine/timer.h>

namespace bowtie
{
namespace sprite_script_interface
{

int set_position(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	auto x = lua_tonumber(lua, 2);
	auto y = lua_tonumber(lua, 3);
	sprite.set_position(Vector2((float)x, (float)y));
	return 0;
}

int position(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	lua_pushnumber(lua, sprite.position().x);
	lua_pushnumber(lua, sprite.position().y);
	return 2;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "set_position", set_position },
		{ "position", position }
	};

	script_interface::register_interface(lua, "Sprite", functions, 2);
}

} // namespace sprite_script_interface
} // namespace bowtie