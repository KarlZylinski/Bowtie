#include "render_interface.h"

#include <cassert>
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

RenderResourceData RenderInterface::create_render_resource(RenderResourceData::Type type)
{
	RenderResourceData rr = { type, _renderer.create_handle(), 0 };
	return rr;
}

RendererCommand RenderInterface::create_command(RendererCommand::Type type)
{
	RendererCommand command;
	memset(&command, 0, sizeof(RendererCommand));
	command.type = type;
	return command;
}

bool RenderInterface::active() const
{
	return _renderer.active();
}

void RenderInterface::dispatch(const RendererCommand& command)
{
	_renderer.add_renderer_command(command);
}

void RenderInterface::load_resource(RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	RendererCommand rc;

	auto copied_resource = (RenderResourceData*)_allocator.allocate(sizeof(RenderResourceData));
	memcpy(copied_resource, &resource, sizeof(RenderResourceData));
	rc.data = copied_resource;

	rc.type = RendererCommand::LoadResource;
	
	switch (resource.type)
	{
		case RenderResourceData::Type::Shader:
			copied_resource->data = _allocator.allocate(sizeof(ShaderResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(ShaderResourceData));
			break;
		default:
			assert(!"Unknown resource data type.");
			break;
	}

	rc.dynamic_data = dynamic_data;
	rc.dynamic_data_size = dynamic_data_size;

	dispatch(rc);
}

RenderFence& RenderInterface::create_fence()
{
	auto fence_command = create_command(RendererCommand::Fence);
	fence_command.data = MAKE_NEW(_allocator, RenderFence);
	dispatch(fence_command);	
	return *(RenderFence*)fence_command.data;
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