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
	sprite.set_position(script_interface::to_vector(lua, 2));
	return 0;
}

int position(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	script_interface::push_vector(lua, sprite.position());
	return 1;
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