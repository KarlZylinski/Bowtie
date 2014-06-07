#include "script_sprite.h"

#include <cmath>

#include <lua.hpp>

#include <engine/sprite.h>
#include <engine/timer.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace sprite_script_interface
{

int set_position(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	sprite.set_position(script_interface::to_vector2(lua, 2));
	return 0;
}

int position(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	script_interface::push_vector2(lua, sprite.position());
	return 1;
}

int set_rect(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	sprite.set_rect(Rect(script_interface::to_vector2u(lua, 2), script_interface::to_vector2u(lua, 3)));
	return 0;
}

int rect(lua_State* lua)
{
	auto& sprite = *(Sprite*)lua_touserdata(lua, 1);
	script_interface::push_vector2(lua, sprite.rect().position);
	script_interface::push_vector2(lua, sprite.rect().size);
	return 2;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "set_position", set_position },
		{ "position", position },
		{ "set_rect", set_rect },
		{ "rect", rect }
	};

	script_interface::register_interface(lua, "Sprite", functions, 4);
}

} // namespace sprite_script_interface
} // namespace bowtie