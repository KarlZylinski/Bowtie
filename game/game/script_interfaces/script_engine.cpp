#include "script_engine.h"

#include <lua.hpp>

#include <engine/engine.h>
#include <engine/texture.h>

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
	lua_pushlightuserdata(lua, s_engine);
	return 1;
}

int load_resource(lua_State* lua)
{
	auto resource = resource_store::load(&s_engine->resource_store, resource_store::resource_type_from_string(lua_tostring(lua, 1)), lua_tostring(lua, 2));
	lua_pushlightuserdata(lua, option::get_value_or_null(resource));
	return 1;
}

int reload_resource(lua_State* lua)
{
	resource_store::reload(&s_engine->resource_store, resource_store::resource_type_from_string(lua_tostring(lua, 1)), lua_tostring(lua, 2));
	return 0;
}

int set_default_resource(lua_State* lua)
{
	auto resource_type_str = lua_tostring(lua, 1);
	auto resource_type = resource_store::resource_type_from_string(resource_type_str);
	auto resource_name = lua_tostring(lua, 2);
	auto resource_hash = hash_str(resource_name);
	
	auto resource_store = &s_engine->resource_store;
	auto resource = resource_store::get(resource_store, resource_type, resource_hash);

	if (resource.is_some)
		resource_store::set_default(resource_store, resource_type, resource.value);

	return 0;
}

int keyboard_pressed(lua_State* lua)
{
	lua_pushboolean(lua, keyboard::key_pressed(&s_engine->keyboard, keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_released(lua_State* lua)
{
	lua_pushboolean(lua, keyboard::key_released(&s_engine->keyboard, keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_held(lua_State* lua)
{
	lua_pushboolean(lua, keyboard::key_held(&s_engine->keyboard, keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

void load(lua_State* lua, Engine& e)
{
	s_engine = &e;
	
	const interface_function engine_functions[] = {
		{ "engine", engine },
		{ "set_default_resource", set_default_resource },
		{ "load_resource", load_resource },
		{ "reload_resource", reload_resource },
	};

	script_interface::register_interface(lua, "Engine", engine_functions, 4);
	
	const interface_function keyboard_functions[] = {
		{ "pressed", keyboard_pressed },
		{ "released", keyboard_released },
		{ "held", keyboard_held }
	};

	script_interface::register_interface(lua, "Keyboard", keyboard_functions, 3);
}

} // namespace engine_script_interface
} // namespace bowtie