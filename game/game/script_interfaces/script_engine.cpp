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
	auto resource_type_str = lua_tostring(lua, 1);
	auto resource_type = ResourceManager::resource_type_from_string(resource_type_str);
	
	auto& resource_manager = s_engine->resource_manager();

	switch(resource_type) {
		case ResourceManager::RT_Image:
			resource_manager.load_image(lua_tostring(lua, 2));
			break;
		
		case ResourceManager::RT_Shader:
			resource_manager.load_shader(lua_tostring(lua, 2));
			break;

		case ResourceManager::RT_Sprite:
			resource_manager.load_sprite_prototype(lua_tostring(lua, 2));
			break;
			
		case ResourceManager::RT_Texture:
			resource_manager.load_texture(lua_tostring(lua, 2));
			break;
	}

	return 0;
}

int set_default_resource(lua_State* lua)
{
	auto resource_type_str = lua_tostring(lua, 1);
	auto resource_type = ResourceManager::resource_type_from_string(resource_type_str);
	auto resource_name = lua_tostring(lua, 2);
	auto resource_hash = murmur_hash_64(resource_name, strlen32(resource_name), 0);
	
	auto& resource_manager = s_engine->resource_manager();
	resource_manager.set_default(resource_type, resource_manager.get(resource_type, resource_hash));

	return 0;
}

int keyboard_pressed(lua_State* lua)
{
	lua_pushboolean(lua, s_engine->keyboard().key_pressed(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_released(lua_State* lua)
{
	lua_pushboolean(lua, s_engine->keyboard().key_released(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

int keyboard_held(lua_State* lua)
{
	lua_pushboolean(lua, s_engine->keyboard().key_held(Keyboard::key_from_string(lua_tostring(lua, 1))));
	return 1;
}

void load(lua_State* lua, Engine& e)
{
	s_engine = &e;
	
	const interface_function engine_functions[] = {
		{ "engine", engine },
		{ "set_default_resource", set_default_resource },
		{ "load_resource", load_resource }
	};

	script_interface::register_interface(lua, "Engine", engine_functions, 3);
	
	const interface_function keyboard_functions[] = {
		{ "pressed", keyboard_pressed },
		{ "released", keyboard_released },
		{ "held", keyboard_held }
	};

	script_interface::register_interface(lua, "Keyboard", keyboard_functions, 3);
}

} // namespace engine_script_interface
} // namespace bowtie