#include "script_interface_helpers.h"

#include <cassert>

#include <lua.hpp>

#include "script_console.h"

namespace bowtie
{
namespace script_interface
{

void register_interface(lua_State* lua, const char* interface_name, const interface_function* functions, unsigned num_functions)
{
	lua_newtable(lua);

	for (unsigned i = 0; i < num_functions; ++i)
	{
		auto func = functions[i];

		lua_pushcfunction(lua, func.function);
		lua_setfield(lua, -2, func.name);
	}

	lua_setglobal(lua, interface_name);
}

double get_field(lua_State* lua, int index, const char *key)
{
    lua_pushstring(lua, key);
    lua_gettable(lua, index);

    if (!lua_isnumber(lua, -1))
		return 0;

    auto result = lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return result;
}

Vector2 to_vector2(lua_State* lua, int index)
{
	assert(lua_istable(lua, index));

	auto x_value = (float)get_field(lua, index, "x");
	auto y_value = (float)get_field(lua, index, "y");

	return Vector2(x_value, y_value);
}

Vector2u to_vector2u(lua_State* lua, int index)
{
	assert(lua_istable(lua, index));

	auto x_value = (unsigned)get_field(lua, index, "x");
	auto y_value = (unsigned)get_field(lua, index, "y");

	return Vector2u(x_value, y_value);
}

void push_vector2(lua_State* lua, const Vector2& v)
{
	lua_getglobal(lua, "Vector2");
	lua_pushnumber(lua, v.x);
	lua_pushnumber(lua, v.y);
	int error = lua_pcall(lua, 2, 1, 0);
	script_interface::check_errors(lua, error);
}

void push_vector2(lua_State* lua, const Vector2u& v)
{
	lua_getglobal(lua, "Vector2");
	lua_pushnumber(lua, v.x);
	lua_pushnumber(lua, v.y);
	int error = lua_pcall(lua, 2, 1, 0);
	script_interface::check_errors(lua, error);
}

Vector4 to_vector4(lua_State* lua, int index)
{
	assert(lua_istable(lua, index));

	auto x_value = (float)get_field(lua, index, "x");
	auto y_value = (float)get_field(lua, index, "y");
	auto z_value = (float)get_field(lua, index, "z");
	auto w_value = (float)get_field(lua, index, "w");

	return Vector4(x_value, y_value, z_value, w_value);
}

void push_vector4(lua_State* lua, const Vector4& v)
{
	lua_getglobal(lua, "Vector4");
	lua_pushnumber(lua, v.x);
	lua_pushnumber(lua, v.y);
	lua_pushnumber(lua, v.z);
	lua_pushnumber(lua, v.w);
	int error = lua_pcall(lua, 4, 1, 0);
	script_interface::check_errors(lua, error);
}

Color to_color(lua_State* lua, int index)
{
	return to_vector4(lua, index);
}

void push_color(lua_State* lua, const Color& c)
{
	push_vector4(lua, c);
}

bool check_errors(lua_State* lua, int error)
{
	if (error == 0)
		return true;

	auto error_str = lua_tostring(lua, -1);
	console::write(error_str);
	return false;
}

} // namespace script_interface
} // namespace bowtie