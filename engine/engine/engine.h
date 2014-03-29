#pragma once

#include <foundation/vector2u.h>
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
	void update();
	void resize(const Vector2u& resolution);

private:
	Allocator& _allocator;
	RenderInterface& _render_interface;
	ResourceManager _resource_manager;
	ResourceHandle _test_shader;
	Image* _test_image;
	Sprite _test_sprite;

	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}