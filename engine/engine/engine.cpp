#include "engine.h"

#include <cmath>
#include <cstring>
#include <stdio.h>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/string_utils.h>

#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"
#include "sprite.h"
#include "texture.h"
#include "timer.h"
#include "world.h"


namespace bowtie
{

Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface),
	_resource_manager(allocator, render_interface), _game(allocator, *this)
{
	timer::start();

	_resource_manager.load_shader("test_shader_vs.glsl", "test_shader_fs.glsl");
}

Engine::~Engine()
{
	if (_game.initialized())
		_game.deinit();
}

void Engine::update()
{
	if (!_render_interface.is_setup())
		return;

	if (!_game.initialized())
		_game.init();
				
	float time_elapsed = timer::counter();
	float dt = time_elapsed - _time_elapsed_previous_frame;
	_time_elapsed_previous_frame = time_elapsed;

	_time_since_start += dt;

	_render_interface.wait_for_fence(_render_interface.create_fence());

	_game.update(dt);
}

void Engine::resize(const Vector2u& resolution)
{
	_render_interface.resize(resolution);
}

RenderInterface& Engine::render_interface()
{
	return _render_interface;
}

World* Engine::create_world()
{
	auto world = MAKE_NEW(_allocator, World, _allocator, _render_interface, _resource_manager);
	_render_interface.create_render_world(*world);
	return world;
}

void Engine::destroy_world(World& world)
{
	MAKE_DELETE(_allocator, World, &world);
}

}