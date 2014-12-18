#pragma once
#include <base/vector2.h>
#include <base/vector2i.h>
#include <base/vector4.h>
#include <engine/entity/entity.h>

struct lua_State;
typedef int(*lua_CFunction) (lua_State *L);

namespace bowtie
{

struct World;

struct interface_function {
    const char* name;
    lua_CFunction function;
};

namespace script_interface
{

bool check_errors(lua_State* lua, int32 error);
void push_color(lua_State* lua, const Color* c);
void push_vector2(lua_State* lua, const Vector2* v);
void push_vector2(lua_State* lua, const Vector2i* v);
void push_vector4(lua_State* lua, const Vector4* v);
void push_entity(lua_State* lua, Entity entity);
void register_interface(lua_State* lua, const char* interface_name, const interface_function* functions, uint32 num_functions);
Color to_color(lua_State* lua, int32 index);
Vector2 to_vector2(lua_State* lua, int32 index);
Vector2i to_vector2i(lua_State* lua, int32 index);
Vector4 to_vector4(lua_State* lua, int32 index);
uint64 to_hash(lua_State* lua, int32 index);
Entity to_entity(lua_State* lua, int32 index);
bool is_color(lua_State* lua, int32 index);
bool is_vector2(lua_State* lua, int32 index);
bool is_vector4(lua_State* lua, int32 index);

}

}