#include "script_engine.h"

#include <lua.hpp>

#include <engine/engine.h>

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
	assert(s_engine != nullptr && "s_engine not intitialized");
	lua_pushlightuserdata(lua, s_engine);
	return 1;
}

void load(lua_State* lua, Engine& e)
{
	s_engine = &e;
	
	const interface_function functions[] = {
		{ "engine", engine }
	};

	script_interface::register_interface(lua, "Engine", functions, 1);
}

} // namespace engine_script_interface
} // namespace bowtie