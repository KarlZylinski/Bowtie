#include "script_text.h"

#include <lua.hpp>
#include <engine/rect.h>
#include <engine/rectangle_geometry.h>
#include <engine/world.h>
#include "script_interface_helpers.h"

namespace bowtie
{
namespace rectangle_script_interface
{

int spawn(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto position = script_interface::to_vector2(lua, 2);
	auto size = script_interface::to_vector2(lua, 3);
	auto color = script_interface::to_color(lua, 4);	
	int depth = 0;

	if (lua_isnumber(lua, 5))
		depth = (int)lua_tonumber(lua, 5);

	lua_pushlightuserdata(lua, world.spawn_rectangle(Rect(position, size), color, depth));

	return 1;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "spawn", spawn }
	};

	script_interface::register_interface(lua, "Rectangle", functions, 1);
}

} // namespace rectangle_script_interface
} // namespace bowtie