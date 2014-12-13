#pragma once

struct lua_State;

namespace bowtie
{

struct Allocator;

namespace transform_component_script_interface
{

void load(lua_State* lua, Allocator* allocator);

}

}