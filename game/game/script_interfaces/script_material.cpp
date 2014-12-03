#include "script_material.h"

#include <engine/material.h>
#include <engine/render_interface.h>
#include <engine/render_resource_handle.h>
#include <foundation/murmur_hash.h>
#include <lua.hpp>
#include "script_interface_helpers.h"

namespace bowtie
{
namespace material_script_interface
{
	
namespace 
{
	RenderInterface* s_render_interface = nullptr;
}

int set_uniform_value(lua_State* lua)
{
	auto material = (Material*)lua_touserdata(lua, 1);
	auto name = hash_str(lua_tostring(lua, 2));
	auto value = (float)lua_tonumber(lua, 3);
	auto command = render_interface::create_command(*s_render_interface, RendererCommand::SetUniformValue);
	SetUniformValueData& suvd = *(SetUniformValueData*)command.data;
	suvd.material = material->render_handle;
	suvd.type = uniform::Float;
	suvd.uniform_name = name;
	command.data = &suvd;
	render_interface::dispatch(*s_render_interface, command, &value, sizeof(float));
	return 0;
}

void load(lua_State* lua, RenderInterface& render_interface)
{
	s_render_interface = &render_interface;

	const interface_function functions[] = {
		{ "set_uniform_value", set_uniform_value }
	};

	script_interface::register_interface(lua, "Material", functions, 1);
}

} // namespace material_script_interface
} // namespace bowtie
