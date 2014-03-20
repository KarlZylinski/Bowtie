#include "render_interface.h"

#include <condition_variable>
#include <mutex>

#include <foundation/memory.h>

#include "render_fence.h"
#include "renderer.h"

namespace bowtie
{

RenderInterface::RenderInterface(Renderer& renderer, Allocator& allocator) : _allocator(allocator), _renderer(renderer)
{
}

bool RenderInterface::active() const
{
	return _renderer.active();
}

void RenderInterface::dispatch(const RendererCommand& command)
{
	RendererCommand rc = command;
	_renderer.add_renderer_command(rc);
}

RenderFence& RenderInterface::create_fence()
{
	RendererCommand rc;
	memset(&rc, 0, sizeof(RendererCommand));

	rc.type = RendererCommand::Fence;
	RenderFence* fence = MAKE_NEW(_allocator, RenderFence);
	rc.data = fence;
	
	dispatch(rc);
	
	return *fence;
}

void RenderInterface::wait_for_fence(RenderFence& fence)
{
	{
		std::unique_lock<std::mutex> lock(fence.mutex);
		fence.fence_processed.wait(lock, [&fence] { return fence.processed; });
	}
	MAKE_DELETE(_allocator, RenderFence, &fence);
}

}