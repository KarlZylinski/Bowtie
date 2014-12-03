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

class Engine
{
public:
	Engine(Allocator& allocator, RenderInterface& render_interface);
	~Engine();
	World* create_world();
	void destroy_world(World& world);
	const Keyboard& keyboard() const;
	void key_pressed(keyboard::Key key);
	void key_released(keyboard::Key key);
	EntityManager& entity_manager();
	RenderInterface& render_interface();
	ResourceManager& resource_manager();
	void resize(const Vector2u& resolution);
	void update();

private:
	Allocator& _allocator;
	EntityManager _entity_manager;
	Keyboard _keyboard;
	Game _game;
	ResourceManager _resource_manager;
	RenderInterface& _render_interface;

	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}