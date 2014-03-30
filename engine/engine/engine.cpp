#include "engine.h"

#include <cmath>
#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/string_utils.h>

#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"
#include "sprite.h"
#include "timer.h"

namespace bowtie
{

Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface), _resource_manager(allocator, render_interface)
{
	timer::start();

	_test_shader = _resource_manager.load_shader("test_shader_vs.glsl", "test_shader_fs.glsl");
	_test_image = _resource_manager.load_image("beer.bmp");
	_test_texture = _render_interface.create_texture(*_test_image);
	_test_sprite = _render_interface.create_sprite(*_test_texture);
}

void Engine::update()
{
	if (!_render_interface.is_setup())
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
	rwd.test_sprite = _test_sprite.render_handle();
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

void Engine::resize(const Vector2u& resolution)
{
	_render_interface.resize(resolution);
}

}