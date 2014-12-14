#include "script_interface_helpers.h"
#include <cassert>
#include <lua.hpp>
#include "script_console.h"
#include <foundation/murmur_hash.h>
#include <foundation/vector2u.h>

namespace bowtie
{

namespace
{
real64 get_field(lua_State* lua, int32 index, const char *key)
{
    lua_pushstring(lua, key);
    lua_gettable(lua, index);

    if (!lua_isnumber(lua, -1))
		return 0;

	auto result = lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return result;
}

bool is_field(lua_State* lua, int32 index, const char *key)
{
	lua_pushstring(lua, key);
	lua_gettable(lua, index);
	bool result = lua_isnumber(lua, -1) == 1;
	lua_pop(lua, 1);
	return result;
}
}

namespace script_interface
{

bool check_errors(lua_State* lua, int32 error)
{
	if (error == 0)
		return true;

	auto error_str = lua_tostring(lua, -1);
	console::write(error_str);
	return false;
}

void push_color(lua_State* lua, const Color* c)
{
	push_vector4(lua, c);
}

void push_vector2(lua_State* lua, const Vector2* v)
{
	lua_getglobal(lua, "Vector2");
	lua_pushnumber(lua, v->x);
	lua_pushnumber(lua, v->y);
	int32 error = lua_pcall(lua, 2, 1, 0);
	script_interface::check_errors(lua, error);
}

void push_vector2(lua_State* lua, const Vector2i* v)
{
	lua_getglobal(lua, "Vector2");
	lua_pushnumber(lua, v->x);
	lua_pushnumber(lua, v->y);
	int32 error = lua_pcall(lua, 2, 1, 0);
	script_interface::check_errors(lua, error);
}

void push_vector2(lua_State* lua, const Vector2u* v)
{
	lua_getglobal(lua, "Vector2");
	lua_pushnumber(lua, v->x);
	lua_pushnumber(lua, v->y);
	int32 error = lua_pcall(lua, 2, 1, 0);
	script_interface::check_errors(lua, error);
}

void push_vector4(lua_State* lua, const Vector4* v)
{
	lua_getglobal(lua, "Vector4");
	lua_pushnumber(lua, v->x);
	lua_pushnumber(lua, v->y);
	lua_pushnumber(lua, v->z);
	lua_pushnumber(lua, v->w);
	int32 error = lua_pcall(lua, 4, 1, 0);
	script_interface::check_errors(lua, error);
}

void push_entity(lua_State* lua, Entity entity, World* world)
{
	lua_newtable(lua);
	lua_pushnumber(lua, entity);
	lua_rawseti(lua, -2, 1);
	lua_pushlightuserdata(lua, world);
	lua_rawseti(lua, -2, 2);
}

void register_interface(lua_State* lua, const char* interface_name, const interface_function* functions, uint32 num_functions)
{
	lua_newtable(lua);

	for (uint32 i = 0; i < num_functions; ++i)
	{
		auto func = functions[i];

		lua_pushcfunction(lua, func.function);
		lua_setfield(lua, -2, func.name);
	}

	lua_setglobal(lua, interface_name);
}

Color to_color(lua_State* lua, int32 index)
{
	return to_vector4(lua, index);
}

Vector2 to_vector2(lua_State* lua, int32 index)
{
	assert(lua_istable(lua, index));

	auto x_value = (real32)get_field(lua, index, "x");
	auto y_value = (real32)get_field(lua, index, "y");

	return vector2::create(x_value, y_value);
}

Vector2i to_vector2i(lua_State* lua, int32 index)
{
	assert(lua_istable(lua, index));

	auto x_value = (int)get_field(lua, index, "x");
	auto y_value = (int)get_field(lua, index, "y");

	return vector2i::create(x_value, y_value);
}

Vector2u to_vector2u(lua_State* lua, int32 index)
{
	assert(lua_istable(lua, index));

	auto x_value = (uint32)get_field(lua, index, "x");
	auto y_value = (uint32)get_field(lua, index, "y");

	return vector2u::create(x_value, y_value);
}

Vector4 to_vector4(lua_State* lua, int32 index)
{
	assert(lua_istable(lua, index));

	auto x_value = (real32)get_field(lua, index, "x");
	auto y_value = (real32)get_field(lua, index, "y");
	auto z_value = (real32)get_field(lua, index, "z");
	auto w_value = (real32)get_field(lua, index, "w");
	
	return vector4::create(x_value, y_value, z_value, w_value);
}

uint64 to_hash(lua_State* lua, int32 index)
{
	assert(lua_isstring(lua, index));

	return hash_str(lua_tostring(lua, index));
}

SpawnedEntity to_entity(lua_State* lua, int32 index)
{
	SpawnedEntity e;
	lua_pushnumber(lua, 1);
	lua_gettable(lua, index);
	e.entity = (Entity)lua_tonumber(lua, -1);
	lua_pop(lua, 1);
	lua_pushnumber(lua, 2);
	lua_gettable(lua, index);
	e.world = (World*)lua_touserdata(lua, -1);
	lua_pop(lua, 1);
	return e;
}

bool is_color(lua_State* lua, int32 index)
{
	return is_vector4(lua, index);
}

bool is_vector2(lua_State* lua, int32 index)
{
	return lua_istable(lua, index)
		&& is_field(lua, index, "x")
		&& is_field(lua, index, "y");
}

bool is_vector4(lua_State* lua, int32 index)
{
	return lua_istable(lua, index)
		&& is_field(lua, index, "x")
		&& is_field(lua, index, "y")
		&& is_field(lua, index, "z")
		&& is_field(lua, index, "w");
}

} // namespace script_interface
} // namespace bowtie
