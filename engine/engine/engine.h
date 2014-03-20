#pragma once

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

	Engine(const Engine&);
	Engine& operator=(const Engine&);
};

}