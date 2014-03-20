#pragma once

namespace bowtie
{

class Allocator;
class Renderer;
struct RenderFence;
struct RendererCommand;
class RenderInterface
{
public:
	RenderInterface(Renderer& renderer, Allocator& allocator);
	
	bool active() const;
	void dispatch(const RendererCommand& command);

	RenderFence& create_fence();
	void wait_for_fence(RenderFence& fence);

private:
	Allocator& _allocator;
	Renderer& _renderer;

	RenderInterface(const RenderInterface&);
	RenderInterface& operator=(const RenderInterface&);
};

}