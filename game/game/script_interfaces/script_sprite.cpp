#include "script_sprite.h"

#include <cmath>

#include <lua.hpp>

#include <engine/drawable.h>
#include <engine/sprite_geometry.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace sprite_script_interface
{

SpriteGeometry& geometry(lua_State* lua, int i)
{
	auto& drawable = *(Drawable*)lua_touserdata(lua, i);
	return *(SpriteGeometry*)&drawable.geometry();
}

int set_rect(lua_State* lua)
{
	auto& sprite_geometry = geometry(lua, 1);
	sprite_geometry.set_rect(Rect(script_interface::to_vector2(lua, 2), script_interface::to_vector2(lua, 3)));
	return 0;
}

int rect(lua_State* lua)
{
	auto& sprite_geometry = geometry(lua, 1);
	script_interface::push_vector2(lua, sprite_geometry.rect().position);
	script_interface::push_vector2(lua, sprite_geometry.rect().size);
	return 2;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "set_rect", set_rect },
		{ "rect", rect }
	};

	script_interface::register_interface(lua, "Sprite", functions, 2);
}

} // namespace sprite_script_interface
} // namespace bowtie