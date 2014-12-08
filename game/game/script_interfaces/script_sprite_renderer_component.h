#pragma once

struct lua_State;

namespace bowtie
{

struct Allocator;

namespace sprite_renderer_component_script_interface
{

void load(lua_State* lua, Allocator& allocator);

}

}