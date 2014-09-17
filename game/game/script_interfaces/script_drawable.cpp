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

int color(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	script_interface::push_vector4(lua, drawable.color());
	return 1;
}

int depth(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	lua_pushnumber(lua, drawable.depth());
	return 1;
}

int material(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	lua_pushlightuserdata(lua, drawable.material());
	return 1;
}

int pivot(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	script_interface::push_vector2(lua, drawable.pivot());
	return 1;
}

int position(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	script_interface::push_vector2(lua, drawable.position());
	return 1;
}

int rotation(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	lua_pushnumber(lua, drawable.rotation());
	return 1;
}

int set_color(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_color(script_interface::to_vector4(lua, 2));
	return 0;
}

int set_depth(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_depth((float)lua_tonumber(lua, 2));
	return 0;
}

int set_material(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_material((Material*)lua_touserdata(lua, 2));
	return 0;
}

int set_pivot(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_pivot(script_interface::to_vector2i(lua, 2));
	return 0;
}

int set_position(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	drawable.set_position(script_interface::to_vector2(lua, 2));
	return 0;
}

int set_rotation(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	auto rotation = (float)lua_tonumber(lua, 2);
	drawable.set_rotation(rotation);
	return 0;
}

int set_parent(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	auto parent = (Drawable*)lua_touserdata(lua, 2);
	drawable.set_parent(parent);
	return 0;
}

int parent(lua_State* lua)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, 1);
	lua_pushlightuserdata(lua, drawable.parent());
	return 1;
}

int unspawn(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto& drawable = *(Drawable*)lua_touserdata(lua, 2);
	world.unspawn(drawable);
	return 0;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "color", color },
		{ "depth", depth },
		{ "material", material },
		{ "pivot", pivot },
		{ "position", position },
		{ "rotation", rotation },
		{ "set_color", set_color },
		{ "set_depth", set_depth },
		{ "set_material", set_material },
		{ "set_pivot", set_pivot },
		{ "set_position", set_position },
		{ "set_rotation", set_rotation },
		{ "set_parent", set_parent },
		{ "parent", parent },
		{ "unspawn", unspawn }
	};

	script_interface::register_interface(lua, "Drawable", functions, 15);
}

} // namespace drawable_script_interface
} // namespace bowtie