#include "script_world.h"

#include <lua.hpp>

#include <engine/timer.h>

#include "script_interface_helpers.h"

namespace bowtie
{
namespace time_script_interface
{

int time(lua_State* lua)
{
	auto counter = timer::counter();
	lua_pushnumber(lua, counter);
	return 1;
}


void load(lua_State* lua)
{
	const interface_function functions[] = {
		{ "time", time }
	};

	script_interface::register_interface(lua, "Time", functions, 1);
}

} // namespace time_script_interface
} // namespace bowtie