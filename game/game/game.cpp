#include "game.h"

#include <cassert>
#include <lua.hpp>
#include "script_interfaces/script_console.h"
#include "script_interfaces/script_interface_helpers.h"
#include "script_interfaces/script_drawable.h"
#include "script_interfaces/script_sprite.h"
#include "script_interfaces/script_entity.h"
#include "script_interfaces/script_world.h"
#include "script_interfaces/script_engine.h"
#include "script_interfaces/script_rectangle.h"
#include "script_interfaces/script_time.h"
#include "script_interfaces/script_text.h"
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
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/console.lua"));
}

Game::Game(Allocator& allocator, Engine& engine, RenderInterface& render_interface) : _allocator(allocator), _lua(luaL_newstate()), _engine(engine), _initialized(false)
{
	luaL_openlibs(_lua);
	engine_script_interface::load(_lua, engine);
	world_script_interface::load(_lua);
	entity_script_interface::load(_lua, engine.entity_manager());
	time_script_interface::load(_lua);
	drawable_script_interface::load(_lua);
	sprite_script_interface::load(_lua);
	text_script_interface::load(_lua);
	rectangle_script_interface::load(_lua);
	material_script_interface::load(_lua, render_interface);
	
	// Components
	sprite_renderer_component_script_interface::load(_lua, allocator);
	transform_component_script_interface::load(_lua, allocator);

	load_shared_libs(_lua);
	load_main(_lua);
}

Game::~Game()
{
	lua_close(_lua);
}

void Game::init()
{
	assert(!_initialized && "init() has already been called once");

	console::init(_lua);
	init_game(_lua);
	_initialized = true;
}

void Game::update(float dt)
{
	if (!_initialized)
		return;

	update_game(_lua, dt);
	console::update(dt);
}

void Game::draw()
{
	if (!_initialized)
		return;

	draw_game(_lua);
	console::draw();
}

void Game::deinit()
{
	assert(_initialized && "init() hasn't been called");

	deinit_game(_lua);
	_initialized = false;
	lua_gc(_lua, LUA_GCCOLLECT, 0);
}

bool Game::initialized() const
{
	return _initialized;
}

};