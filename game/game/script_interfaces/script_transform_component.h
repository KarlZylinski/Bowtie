#pragma once

struct lua_State;

namespace bowtie
{

class Allocator;

namespace transform_component_script_interface
{

void load(lua_State* lua, Allocator& allocator);

}

}