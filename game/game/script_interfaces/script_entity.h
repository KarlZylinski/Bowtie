#pragma once

struct lua_State;

namespace bowtie
{

struct EntityManager;
class Allocator;

namespace entity_script_interface
{

void load(lua_State* lua, EntityManager& manager, Allocator& allocator);

}

}