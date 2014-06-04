#include "game.h"

#include <cassert>

#include <lua.hpp>

#include "script_interfaces/script_sprite.h"
#include "script_interfaces/script_world.h"
#include "script_interfaces/script_engine.h"
#include "script_interfaces/script_time.h"

namespace bowtie
{

void load_libs(lua_State* lua)
{
	luaopen_io(lua);
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);
}

void load_main(lua_State* lua)
{
	int error = luaL_dofile(lua, "main.lua");

	if (error != 0)
		assert(!"Could not load main.lua.");
}

void init_game(lua_State* lua)
{
	lua_getglobal(lua, "init");
	int error = lua_pcall(lua, 0, 0, 0);

	if (error != 0)
		assert(!"Failed to call init in main.lua.");
}

void update_game(lua_State* lua, float dt)
{
	lua_getglobal(lua, "update");
	lua_pushnumber(lua, dt);

	int error = lua_pcall(lua, 1, 0, 0);

	if (error != 0)
		assert(!"Failed to call update in main.lua.");
}

void deinit_game(lua_State* lua)
{
	lua_getglobal(lua, "deinit");
	int error = lua_pcall(lua, 0, 0, 0);

	if (error != 0)
		assert(!"Failed to call deinit in main.lua.");
}

Game::Game(Allocator& allocator, Engine& engine) : _allocator(allocator), _lua(luaL_newstate()), _engine(engine), _initialized(false)
{
	load_libs(_lua);
	load_main(_lua);

	engine_script_interface::load(_lua, engine);
	time_script_interface::load(_lua);
	sprite_script_interface::load(_lua);
	world_script_interface::load(_lua);
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
}

void Game::update(float dt)
{
	update_game(_lua, dt);
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