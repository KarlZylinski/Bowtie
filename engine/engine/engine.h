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

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}