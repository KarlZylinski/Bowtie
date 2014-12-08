#pragma once

#include <foundation/vector2u.h>
#include <game/game.h>

#include "keyboard.h"
#include "resource_manager.h"
#include "entity/entity_manager.h"

namespace bowtie
{

class Allocator;
struct RenderInterface;
struct World;

struct Engine
{
	Allocator* allocator;
	EntityManager entity_manager;
	ResourceManager resource_manager;
	RenderInterface* render_interface;
	Keyboard keyboard;
	Game _game;
	float _time_elapsed_previous_frame;
	float _time_since_start;
};

namespace engine
{
	void init(Engine& e, Allocator& allocator, RenderInterface& render_interface);
	void deinit(Engine& e);
	World* create_world(Engine& e);
	void destroy_world(Engine& e, World& world);
	const Keyboard& keyboard(const Engine& e);
	void key_pressed(Engine& e, keyboard::Key key);
	void key_released(Engine& e, keyboard::Key key);
	void resize(Engine& e, const Vector2u& resolution);
	void update(Engine& e);

}

}