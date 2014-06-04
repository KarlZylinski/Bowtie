#pragma once

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

}

}