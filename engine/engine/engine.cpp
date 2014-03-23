#include "engine.h"

#include <cmath>
#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/string_utils.h>

#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"
#include "timer.h"

namespace bowtie
{

RenderResourceHandle create_shader(Allocator& allocator, RenderInterface& render_interface, const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	char* vertex_shader_source = file::load(vertex_shader_filename, allocator);
	char* fragment_shader_source = file::load(fragment_shader_filename, allocator);
	
	ShaderResourceData srd;
	unsigned shader_dynamic_data_size = strlen32(vertex_shader_source) + strlen32(fragment_shader_source) + 2;
	unsigned shader_dynamic_data_offset = 0;
	void* shader_resource_dynamic_data = allocator.allocate(shader_dynamic_data_size);

	srd.vertex_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)shader_resource_dynamic_data, vertex_shader_source);
	shader_dynamic_data_offset += strlen32(vertex_shader_source) + 1;

	srd.fragment_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), fragment_shader_source);

	RenderResourceData shader_resource = render_interface.create_render_resource(RenderResourceData::Shader);
	shader_resource.data = &srd;
	
	render_interface.load_resource(shader_resource, shader_resource_dynamic_data, shader_dynamic_data_size);

	allocator.deallocate(vertex_shader_source);
	allocator.deallocate(fragment_shader_source);

	return shader_resource.handle;
}


Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface)
{
	timer::start();

	_test_shader = create_shader(allocator, render_interface, "test_shader_vs.glsl", "test_shader_fs.glsl");
}

void Engine::update()
{
	if (!_render_interface.active())
		return;
				
	float time_elapsed = timer::counter();
	float dt = time_elapsed - _time_elapsed_previous_frame;
	_time_elapsed_previous_frame = time_elapsed;

	_time_since_start += dt;

	_render_interface.wait_for_fence(_render_interface.create_fence());
	
	auto render_world_command = _render_interface.create_command(RendererCommand::RenderWorld);
	
	View view(Vector2(640,480), Vector2(0,0));
	view.set_position(cos(_time_since_start) * 100.0f,sin(_time_since_start) * 100.0f);

	RenderWorldData& rwd = *(RenderWorldData*)_allocator.allocate(sizeof(RenderWorldData));
	rwd.view = view;
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

void Engine::resize(const Vector2u& resolution)
{
	_render_interface.resize(resolution);
}

}