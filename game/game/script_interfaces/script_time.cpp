#include "script_world.h"
#include <lua.hpp>
#include <engine/timer.h>
#include <engine/engine.h>
#include "script_interface_helpers.h"

namespace bowtie
{

namespace
{
    Engine* s_engine;
}

namespace time_script_interface
{

int32 time(lua_State* lua)
{
    auto counter = s_engine->timer->counter();
    lua_pushnumber(lua, counter);
    return 1;
}

void load(lua_State* lua, Engine* engine)
{
    s_engine = engine;

    const interface_function functions[] = {
        { "time", time }
    };

    script_interface::register_interface(lua, "Time", functions, 1);
}

} // namespace time_script_interface
} // namespace bowtie