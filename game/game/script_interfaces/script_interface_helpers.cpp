#include "script_interface_helpers.h"

#include <lua.hpp>

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

} // namespace script_interface
} // namespace bowtie