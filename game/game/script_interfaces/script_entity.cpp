#include "script_entity.h"

#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <foundation/murmur_hash.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/entity/components/sprite_renderer_component.h>

namespace bowtie
{

namespace entity_script_interface
{

namespace
{
	EntityManager* s_manager = nullptr;
}

int create(lua_State* lua)
{
	auto world = (World*)lua_touserdata(lua, 1);
	script_interface::push_entity(lua, entity_manager::create(*s_manager), world);
	return 1;
}

int destroy(lua_State* lua)
{
	auto entity = script_interface::to_entity(lua, 1);
	entity_manager::destroy(*s_manager, entity.entity);
	return 0;
}

void load(lua_State* lua, EntityManager& manager)
{
	s_manager = &manager;

	const interface_function functions[] = {
		{ "create", create },
		{ "destroy", destroy }
	};

	script_interface::register_interface(lua, "Entity", functions, 2);
}

} // namespace drawable_script_interface
} // namespace bowtie