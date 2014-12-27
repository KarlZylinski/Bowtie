#pragma once

struct lua_State;

namespace bowtie
{

struct EntityManager;

namespace entity_script_interface
{

void load(lua_State* lua, EntityManager* manager);

}

}