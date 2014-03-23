#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
class RenderInterface;

class Engine
{
public:
	Engine(Allocator& allocator, RenderInterface& render_interface);
	void update();

private:
	Allocator& _allocator;
	RenderInterface& _render_interface;
	RenderResourceHandle _test_shader;
	float _time_elapsed_previous_frame;
	float _time_since_start;

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}