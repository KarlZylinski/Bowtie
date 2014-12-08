#include "game.h"

#include <cassert>
#include <lua.hpp>
#include "script_interfaces/script_console.h"
#include "script_interfaces/script_interface_helpers.h"
#include "script_interfaces/script_entity.h"
#include "script_interfaces/script_world.h"
#include "script_interfaces/script_engine.h"
#include "script_interfaces/script_time.h"
#include "script_interfaces/script_material.h"
#include "script_interfaces/script_sprite_renderer_component.h"
#include "script_interfaces/script_transform_component.h"
#include <engine/engine.h>
#include <random>

namespace bowtie
{

void load_main(lua_State* lua)
{
	int error = luaL_dofile(lua, "main.lua");
	script_interface::check_errors(lua, error);
}

void init_game(lua_State* lua)
{
	lua_getglobal(lua, "init");
	int error = lua_pcall(lua, 0, 0, 0);
	script_interface::check_errors(lua, error);
}

void update_game(lua_State* lua, float dt)
{
	lua_getglobal(lua, "update");
	lua_pushnumber(lua, dt);
	int error = lua_pcall(lua, 1, 0, 0);
	script_interface::check_errors(lua, error);
}

void draw_game(lua_State* lua)
{
	lua_getglobal(lua, "draw");
	int error = lua_pcall(lua, 0, 0, 0);
	script_interface::check_errors(lua, error);
}

void deinit_game(lua_State* lua)
{
	lua_getglobal(lua, "deinit");
	int error = lua_pcall(lua, 0, 0, 0);
	script_interface::check_errors(lua, error);
}

void load_shared_libs(lua_State* lua)
{
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/class.lua"));
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/vector2.lua"));
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/vector4.lua"));
}


namespace game
{

void init(Game& g, Allocator& allocator, Engine& engine, RenderInterface& render_interface)
{
	g.started = false;
	g._lua = luaL_newstate();
	luaL_openlibs(g._lua);
	engine_script_interface::load(g._lua, engine);
	world_script_interface::load(g._lua);
	entity_script_interface::load(g._lua, engine.entity_manager);
	time_script_interface::load(g._lua);
	material_script_interface::load(g._lua, render_interface);

	// Components
	sprite_renderer_component_script_interface::load(g._lua, allocator);
	transform_component_script_interface::load(g._lua, allocator);

	load_shared_libs(g._lua);
	load_main(g._lua);
}

void start(Game& g)
{
	console::init(g._lua);
	init_game(g._lua);
	g.started = true;
}

void deinit(Game& g)
{
	deinit_game(g._lua);
	g.started = false;
	lua_gc(g._lua, LUA_GCCOLLECT, 0);
	lua_close(g._lua);
}

void update(Game& g, float dt)
{
	if (!g.started)
		return;

	update_game(g._lua, dt);
	console::update(dt);
}

void draw(Game& g)
{
	if (!g.started)
		return;

	draw_game(g._lua);
	console::draw();
}

}

}
