#include "script_world.h"

#include <cassert>

#include <lua.hpp>

#include <engine/timer.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace vector_script_interface
{

int new_vector(lua_State* lua)
{
	auto num_arguments = lua_gettop(lua);
	
	assert(num_arguments == 0 || num_arguments == 2);

	auto x = num_arguments == 0 ? 0 : lua_tonumber(lua, 1);
	auto y = num_arguments == 0 ? 0 : lua_tonumber(lua, 2);

	script_interface::push_vector(lua, Vector2((float)x, (float)y));
	return 1;
}

int add(lua_State* lua)
{
	auto v1 = script_interface::to_vector(lua, 1);
	auto v2 = script_interface::to_vector(lua, 2);

	script_interface::push_vector(lua, v1 + v2);
	return 1;
}

int sub(lua_State* lua)
{
	auto v1 = script_interface::to_vector(lua, 1);
	auto v2 = script_interface::to_vector(lua, 2);

	script_interface::push_vector(lua, v1 - v2);
	return 1;
}

int mul(lua_State* lua)
{
	auto vec_pos = lua_isnumber(lua, 2) ? 1 : 2;
	auto scalar_pos = lua_isnumber(lua, 2) ? 2 : 1;

	auto v1 = script_interface::to_vector(lua, vec_pos);
	auto s = (float)lua_tonumber(lua, scalar_pos);

	script_interface::push_vector(lua, v1 * s);
	return 1;
}

void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "new", new_vector },
		{ "add", add },
		{ "sub", sub },
		{ "mul", mul }
	};

	script_interface::register_interface(lua, "Vector", functions, 4);
}

} // namespace time_script_interface
} // namespace bowtie