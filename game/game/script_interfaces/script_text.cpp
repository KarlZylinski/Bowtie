#include "script_text.h"

#include <lua.hpp>
#include <engine/text_geometry.h>
#include <engine/world.h>
#include "script_interface_helpers.h"

namespace bowtie
{
namespace text_script_interface
{

int spawn(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto& font = *(Font*)lua_touserdata(lua, 2);
	auto text = lua_tostring(lua, 3);

	lua_pushlightuserdata(lua, world.spawn_text(font, text));

	return 1;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "spawn", spawn }
	};

	script_interface::register_interface(lua, "Text", functions, 1);
}

} // namespace sprite_script_interface
} // namespace bowtie