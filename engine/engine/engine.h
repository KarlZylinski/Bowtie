#pragma once

#include <foundation/keyboard.h>
#include <foundation/vector2u.h>
#include <game/game.h>

#include "keyboard.h"
#include "resource_manager.h"

namespace bowtie
{

class Allocator;
class RenderInterface;
class Drawable;
class World;

class Engine
{
public:
	Engine(Allocator& allocator, RenderInterface& render_interface);
	~Engine();
	void update();
	void resize(const Vector2u& resolution);
	RenderInterface& render_interface();
	World* create_world();
	void destroy_world(World& world);
	void key_pressed(keyboard::Key key);
	void key_released(keyboard::Key key);
	const Keyboard& keyboard() const;
	ResourceManager& resource_manager();

private:
	Allocator& _allocator;
	Keyboard _keyboard;
	RenderInterface& _render_interface;
	ResourceManager _resource_manager;
	Game _game;

	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}