#include "script_entity.h"

#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/entity/components/rectangle_renderer_component.h>

namespace bowtie
{

namespace entity_script_interface
{

namespace
{
	EntityManager* s_manager = nullptr;
	Allocator* s_component_allocator = nullptr;
}

int create(lua_State* lua)
{
	lua_pushnumber(lua, entity_manager::create(*s_manager));
	return 1;
}

int destroy(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	entity_manager::destroy(*s_manager, entity);
	return 0;
}

int add_component(lua_State* lua)
{
	Entity entity = (unsigned)lua_tonumber(lua, 1);
	uint64_t component_name = script_interface::to_hash(lua, 2);
		
	if (component_name == rectangle_renderer_component::name)
	{
		World& world = *(World*)lua_touserdata(lua, 3);
		rectangle_renderer_component::create(world.rectangle_renderer_component(), entity, *s_component_allocator);
	}

	return 0;
}

void load(lua_State* lua, EntityManager& manager, Allocator& allocator)
{
	s_manager = &manager;
	s_component_allocator = &allocator;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy },
		{ "add_component", add_component }
	};

	script_interface::register_interface(lua, "Entity", functions, 3);
}

} // namespace drawable_script_interface
} // namespace bowtie