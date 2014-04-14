#include "script_engine.h"

#include <lua.hpp>

#include <engine/engine.h>

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
	lua_register(lua, "engine", engine);
}

} // namespace engine_script_interface
} // namespace bowtie