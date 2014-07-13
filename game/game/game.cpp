#include "game.h"

#include <cassert>

#include <lua.hpp>

#include "script_interfaces/script_console.h"
#include "script_interfaces/script_interface_helpers.h"
#include "script_interfaces/script_drawable.h"
#include "script_interfaces/script_sprite.h"
#include "script_interfaces/script_world.h"
#include "script_interfaces/script_engine.h"
#include "script_interfaces/script_time.h"

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
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/vector2.lua"));
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/vector4.lua"));
	script_interface::check_errors(lua, luaL_dofile(lua, "shared/console.lua"));
}

Game::Game(Allocator& allocator, Engine& engine) : _allocator(allocator), _lua(luaL_newstate()), _engine(engine), _initialized(false)
{
	luaL_openlibs(_lua);
	load_main(_lua);
	load_shared_libs(_lua);

	engine_script_interface::load(_lua, engine);
	world_script_interface::load(_lua);
	time_script_interface::load(_lua);
	drawable_script_interface::load(_lua);
	sprite_script_interface::load(_lua);
}

Game::~Game()
{
	lua_close(_lua);
}

void Game::init()
{
	assert(!_initialized && "init() has already been called once");

	init_game(_lua);
	_initialized = true;
	console::load(_lua);
}

void Game::update(float dt)
{
	update_game(_lua, dt);

	const char* test_messages[] = {
		"mega dunder 2000 hej då",
		"FAIL FAIL FAIL",
		"massa skiiiiiiiiiiit",
		"dhelalst ears rstareistn arstnu arst",
		"DONDE ESTA"
	};

	if (rand() % 100 < 5)
		console::write(test_messages[rand() % 5]);

	console::update(dt);
}

void Game::draw()
{
	draw_game(_lua);
	console::draw();
}

void Game::deinit()
{
	deinit_game(_lua);
}

bool Game::initialized() const
{
	return _initialized;
}

};