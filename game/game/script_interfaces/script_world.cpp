#include "script_world.h"

#include <lua.hpp>

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/world.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace world_script_interface
{

int create(lua_State* lua)
{
	auto& engine = *(Engine*)lua_touserdata(lua, 1);
	auto world = engine.create_world();
	lua_pushlightuserdata(lua, world);
	return 1;
}

int destroy(lua_State* lua)
{
	auto& engine = *(Engine*)lua_touserdata(lua, 1);
	auto& world = *(World*)lua_touserdata(lua, 2);
	engine.destroy_world(world);
	return 0;
}

int update(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	world.update();

	return 0;
}

int draw(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto view_pos = script_interface::to_vector2(lua, 2);
	auto view_size = script_interface::to_vector2(lua, 3);
	world.draw(View(Rect(view_pos, view_size)));

	return 0;
}

int spawn_sprite(lua_State* lua)
{
	auto& world = *(World*)lua_touserdata(lua, 1);
	auto name = lua_tostring(lua, 2);

	lua_pushlightuserdata(lua, world.spawn_sprite(name));

	return 1;
}

int spawn_text(lua_State* lua)
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
		{ "create", create },
		{ "destroy", destroy },
		{ "update", update },
		{ "draw", draw },
		{ "spawn_sprite", spawn_sprite },
		{ "spawn_text", spawn_text }
	};

	script_interface::register_interface(lua, "World", functions, 6);
}

} // namespace world_script_interface
} // namespace bowtie