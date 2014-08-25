#pragma once

struct lua_State;
class RenderInterface;

namespace bowtie
{

namespace material_script_interface
{

void load(lua_State* lua, RenderInterface& render_interface);

}

}