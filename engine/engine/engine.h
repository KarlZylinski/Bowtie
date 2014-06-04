#pragma once

#include <foundation/vector2u.h>
#include <game/game.h>

#include "resource_manager.h"
#include "sprite.h"

namespace bowtie
{

class Allocator;
class RenderInterface;
class Sprite;
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

private:
	Allocator& _allocator;
	RenderInterface& _render_interface;
	ResourceManager _resource_manager;
	Game _game;

	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}