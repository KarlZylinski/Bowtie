#include "script_drawable.h"

#include <cmath>
#include <lua.hpp>
#include <engine/drawable.h>
#include <engine/world.h>
#include "script_interface_helpers.h"
#include "script_console.h"

namespace bowtie
{
namespace drawable_script_interface
{

int set_position(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_position(script_interface::to_vector2(lua, 2));
	return 0;
}

int position(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	script_interface::push_vector2(lua, drawable.position());
	return 1;
}

int set_color(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_color(script_interface::to_vector4(lua, 2));
	return 0;
}

int set_rotation(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	auto rotation = (float)lua_tonumber(lua, 2);
	drawable.set_rotation(rotation);
	return 0;
}

int color(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	script_interface::push_vector4(lua, drawable.color());
	return 1;
}

int unspawn(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto& drawable = *(Drawable*)lua_touserdata(lua, 2);
	world.unspawn(drawable);
	return 0;
}

int rotation(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	lua_pushnumber(lua, drawable.rotation());
	return 1;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "set_position", set_position },
		{ "position", position },
		{ "set_color", set_color },
		{ "set_rotation", set_rotation },
		{ "color", color },
		{ "unspawn", unspawn },
		{ "rotation", rotation }
	};

	script_interface::register_interface(lua, "Drawable", functions, 7);
}

} // namespace drawable_script_interface
} // namespace bowtie