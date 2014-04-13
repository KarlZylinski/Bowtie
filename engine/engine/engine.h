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

class Engine
{
public:
	Engine(Allocator& allocator, RenderInterface& render_interface);
	~Engine();
	void update();
	void resize(const Vector2u& resolution);

private:
	Allocator& _allocator;
	RenderInterface& _render_interface;
	ResourceManager _resource_manager;
	ResourceHandle _test_shader;
	ResourceHandle _test_render_world;
	Image* _test_image;
	Texture* _test_texture;
	Sprite _test_sprite;
	Game _game;

	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}