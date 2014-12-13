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

void init(Engine* e, Allocator* allocator, RenderInterface* render_interface, Timer* timer)
{
	e->allocator = allocator;
	e->render_interface = render_interface;
	e->timer = timer;
	resource_store::init(&e->resource_store, allocator, render_interface);
	entity_manager::init(&e->entity_manager, allocator);
	memset(&e->keyboard, 0, sizeof(Keyboard));
	e->timer->start();
	game::init(&e->_game, allocator, e, render_interface);
}

void deinit(Engine* e)
{
	game::deinit(&e->_game);
	entity_manager::deinit(&e->entity_manager);
	resource_store::deinit(&e->resource_store);
}

World* create_world(Engine* e)
{
	auto world = (World*)e->allocator->alloc(sizeof(World));
	world::init(world, e->allocator, e->render_interface, &e->resource_store);
	render_interface::create_render_world(e->render_interface, world);
	return world;
}

void destroy_world(Engine* e, World* world)
{
	world::deinit(world);
	e->allocator->dealloc(world);
}

void key_pressed(Engine* e, platform::Key key)
{
	keyboard::set_key_pressed(&e->keyboard, key);
}

void key_released(Engine* e, platform::Key key)
{
	keyboard::set_key_released(&e->keyboard, key);
}

void resize(Engine* e, const Vector2u* resolution)
{
	render_interface::resize(e->render_interface, resolution);
}

void update_and_render(Engine* e)
{
	render_interface::wait_until_idle(e->render_interface);

	if (!e->_game.started)
		game::start(&e->_game);

	float time_elapsed = e->timer->counter();
	float dt = time_elapsed - e->_time_elapsed_previous_frame;
	e->_time_elapsed_previous_frame = time_elapsed;
	e->_time_since_start += dt;
	game::update(&e->_game, dt);
	game::draw(&e->_game);
	auto command = render_interface::create_command(e->render_interface, RendererCommand::CombineRenderedWorlds);
	render_interface::dispatch(e->render_interface, &command);

	if (keyboard::key_pressed(&e->keyboard, platform::Key::F5))
		resource_store::reload_all(&e->resource_store);

	keyboard::reset_pressed_released(&e->keyboard);
}

} // namespace engine

} // namespace bowtie
