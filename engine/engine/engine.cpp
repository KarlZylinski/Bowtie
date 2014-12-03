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
#include "timer.h"
#include "world.h"

namespace bowtie
{

Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _game(allocator, *this, render_interface), _render_interface(render_interface),
	_resource_manager(allocator, render_interface), _time_elapsed_previous_frame(0.0f)
{
	entity_manager::init(_entity_manager, _allocator);
	memset(&_keyboard, 0, sizeof(Keyboard));
	timer::start();
}

Engine::~Engine()
{
	if (_game.initialized())
		_game.deinit();

	entity_manager::deinit(_entity_manager);
}

World* Engine::create_world()
{
	auto world = (World*)_allocator.init(sizeof(World));
	world::init(*world, _allocator, _render_interface, _resource_manager);
	_render_interface.create_render_world(*world);
	return world;
}

void Engine::destroy_world(World& world)
{
	world::deinit(world);
	_allocator.destroy(&world);
}

const Keyboard& Engine::keyboard() const
{
	return _keyboard;
}

void Engine::key_pressed(keyboard::Key key)
{
	keyboard::set_key_pressed(_keyboard, key);
}

void Engine::key_released(keyboard::Key key)
{
	keyboard::set_key_released(_keyboard, key);
}

RenderInterface& Engine::render_interface()
{
	return _render_interface;
}

void Engine::resize(const Vector2u& resolution)
{
	_render_interface.resize(resolution);
}

ResourceManager& Engine::resource_manager()
{
	return _resource_manager;
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
	_render_interface.deallocate_processed_commands(_allocator);

	_game.update(dt);
	_game.draw();
	
	_render_interface.dispatch(_render_interface.create_command(RendererCommand::CombineRenderedWorlds));

	if (keyboard::key_pressed(_keyboard, keyboard::F5))
		_resource_manager.reload_all();

	keyboard::reset_pressed_released(_keyboard);
}

EntityManager& Engine::entity_manager()
{
	return _entity_manager;
}

}
