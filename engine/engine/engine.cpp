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

namespace engine
{

void init(Engine& e, Allocator& allocator, RenderInterface& render_interface)
{
	e.allocator = &allocator;
	e.render_interface = &render_interface;
	e._game.initialized = false; // TODO: Remove hack when Engine is converted.
	resource_manager::init(e.resource_manager, allocator, render_interface);
	entity_manager::init(e.entity_manager, allocator);
	memset(&e.keyboard, 0, sizeof(Keyboard));
	timer::start();
}

void deinit(Engine& e)
{
	if (e._game.initialized)
		game::deinit(e._game);

	entity_manager::deinit(e.entity_manager);
	resource_manager::deinit(e.resource_manager);
}

World* create_world(Engine& e)
{
	auto world = (World*)e.allocator->init(sizeof(World));
	world::init(*world, *e.allocator, *e.render_interface, e.resource_manager);
	render_interface::create_render_world(*e.render_interface, *world);
	return world;
}

void destroy_world(Engine& e, World& world)
{
	world::deinit(world);
	e.allocator->destroy(&world);
}

void key_pressed(Engine& e, keyboard::Key key)
{
	keyboard::set_key_pressed(e.keyboard, key);
}

void key_released(Engine& e, keyboard::Key key)
{
	keyboard::set_key_released(e.keyboard, key);
}

void resize(Engine& e, const Vector2u& resolution)
{
	render_interface::resize(*e.render_interface, resolution);
}

void update(Engine& e)
{
	render_interface::wait_until_idle(*e.render_interface);

	if (!e._game.initialized)
		game::init(e._game, *e.allocator, e, *e.render_interface);

	float time_elapsed = timer::counter();
	float dt = time_elapsed - e._time_elapsed_previous_frame;
	e._time_elapsed_previous_frame = time_elapsed;

	e._time_since_start += dt;

	game::update(e._game, dt);
	game::draw(e._game);

	render_interface::dispatch(*e.render_interface, render_interface::create_command(*e.render_interface, RendererCommand::CombineRenderedWorlds));

	if (keyboard::key_pressed(e.keyboard, keyboard::F5))
		resource_manager::reload_all(e.resource_manager);

	keyboard::reset_pressed_released(e.keyboard);
}

} // namespace engine

} // namespace bowtie
