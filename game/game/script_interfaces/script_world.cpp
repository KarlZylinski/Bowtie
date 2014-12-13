#include "script_world.h"
#include <lua.hpp>
#include <engine/engine.h>
#include <engine/view.h>
#include <engine/world.h>
#include <engine/rect.h>
#include <engine/timer.h>
#include "script_interface_helpers.h"

namespace bowtie
{

namespace
{
	Engine* s_engine;
}

namespace world_script_interface
{

int32 create(lua_State* lua)
{
	auto world = engine::create_world(s_engine);
	lua_pushlightuserdata(lua, world);
	return 1;
}

int32 destroy(lua_State* lua)
{
	auto world = (World*)lua_touserdata(lua, 1);
	engine::destroy_world(s_engine, world);
	return 0;
}

int32 update(lua_State* lua)
{
	world::update((World*)lua_touserdata(lua, 1));
	return 0;
}

int32 draw(lua_State* lua)
{
	auto world = (World*)lua_touserdata(lua, 1);
	auto view_pos = script_interface::to_vector2(lua, 2);
	auto view_size = script_interface::to_vector2(lua, 3);
	Rect view_rect;
	rect::init(&view_rect, &view_pos, &view_size);
	world::draw(world, &view_rect, s_engine->timer->counter());
	return 0;
}

void load(lua_State* lua, Engine* engine)
{
	s_engine = engine;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "update", update },
		{ "draw", draw }
	};

	script_interface::register_interface(lua, "World", functions, 4);
}

} // namespace world_script_interface
} // namespace bowtie