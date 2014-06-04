#include "script_engine.h"

#include <lua.hpp>

#include <engine/engine.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace engine_script_interface
{

namespace 
{
	Engine* s_engine = nullptr;
}

int engine(lua_State* lua)
{
	assert(s_engine != nullptr && "s_engine not intitialized");
	lua_pushlightuserdata(lua, s_engine);
	return 1;
}

int keyboard_pressed(lua_State* lua)
{
	assert(s_engine != nullptr && "s_engine not intitialized");
	lua_pushboolean(lua, s_engine->keyboard().key_pressed(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_released(lua_State* lua)
{
	assert(s_engine != nullptr && "s_engine not intitialized");
	lua_pushboolean(lua, s_engine->keyboard().key_released(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_held(lua_State* lua)
{
	assert(s_engine != nullptr && "s_engine not intitialized");
	lua_pushboolean(lua, s_engine->keyboard().key_held(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

void load(lua_State* lua, Engine& e)
{
	s_engine = &e;
	
	const interface_function engine_functions[] = {
		{ "engine", engine }
	};

	script_interface::register_interface(lua, "Engine", engine_functions, 1);
	
	const interface_function keyboard_functions[] = {
		{ "pressed", keyboard_pressed },
		{ "released", keyboard_released },
		{ "held", keyboard_held }
	};

	script_interface::register_interface(lua, "Keyboard", keyboard_functions, 3);
}

} // namespace engine_script_interface
} // namespace bowtie