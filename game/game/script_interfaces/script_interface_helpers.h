#pragma once

#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/vector4.h>

struct lua_State;
typedef int (*lua_CFunction) (lua_State *L);

struct interface_function {
	const char* name;
	lua_CFunction function;
};

namespace bowtie
{

namespace script_interface
{

void register_interface(lua_State* lua, const char* interface_name, const interface_function* functions, unsigned num_functions);
Vector2 to_vector2(lua_State* lua, int index);
Vector2u to_vector2u(lua_State* lua, int index);
void push_vector2(lua_State* lua, const Vector2& v);
void push_vector2(lua_State* lua, const Vector2u& v);
Vector4 to_vector4(lua_State* lua, int index);
void push_vector4(lua_State* lua, const Vector4& v);
Color to_color(lua_State* lua, int index);
void push_color(lua_State* lua, const Color& c);
bool check_errors(lua_State* lua, int error);

}

}