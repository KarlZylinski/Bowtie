#include "render_interface.h"

#include <cassert>
#include <condition_variable>
#include <mutex>

#include <foundation/memory.h>

#include "irenderer.h"
#include "image.h"
#include "material.h"
#include "render_fence.h"
#include "resource_manager.h"
#include "texture.h"
#include "world.h"

namespace bowtie
{

RenderInterface::RenderInterface(IRenderer& renderer, Allocator& allocator) : _allocator(allocator), _renderer(renderer)
{
}

void RenderInterface::create_texture(Texture& texture)
{
	assert(texture.render_handle == RenderResourceHandle::NotInitialized && "Trying to create already created texture");

	Image& image = *texture.image;

	auto texture_resource = create_render_resource_data(RenderResourceData::Texture);

	auto trd = TextureResourceData();
	trd.handle = create_handle();
	trd.resolution = image.resolution;
	trd.texture_data_dynamic_data_offset = 0;
	trd.texture_data_size = image.data_size;
	trd.pixel_format = image.pixel_format;

	texture_resource.data = &trd;
	create_resource(texture_resource, image.data, image.data_size);
	texture.render_handle = trd.handle;
}

void RenderInterface::create_render_world(World& world)
{
	assert(world.render_handle() == RenderResourceHandle::NotInitialized);

	auto render_world_data = create_render_resource_data(RenderResourceData::World);

	RenderWorldResourceData rwrd;
	rwrd.handle = create_handle();
	render_world_data.data = &rwrd;
	world.set_render_handle(rwrd.handle);
	create_resource(render_world_data);
}

RenderResourceData RenderInterface::create_render_resource_data(RenderResourceData::Type type)
{
	RenderResourceData rr = { type, 0 };
	return rr;
}

RendererCommand RenderInterface::create_command(RendererCommand::Type type)
{
	RendererCommand command;
	memset(&command, 0, sizeof(RendererCommand));
	command.type = type;

	switch (type)
	{
	case RendererCommand::SetUniformValue:
		command.data = _allocator.allocate(sizeof(SetUniformValueData));
		break;
	}

	return command;
}

RenderResourceHandle RenderInterface::create_handle()
{
	return _renderer.create_handle();
}

bool RenderInterface::is_setup() const
{
	return _renderer.is_setup();
}

bool RenderInterface::is_active() const
{
	return _renderer.is_active();
}

void RenderInterface::dispatch(const RendererCommand& command)
{
	_renderer.add_renderer_command(command);
}

void RenderInterface::dispatch(const RendererCommand& command, void* dynamic_data, unsigned dynamic_data_size)
{
	auto command_with_dynamic_data = command;
	command_with_dynamic_data.dynamic_data = _allocator.allocate(dynamic_data_size);
	command_with_dynamic_data.dynamic_data_size = dynamic_data_size;
	memcpy(command_with_dynamic_data.dynamic_data, dynamic_data, dynamic_data_size);
	_renderer.add_renderer_command(command_with_dynamic_data);
}

RendererCommand create_or_update_resource_renderer_command(Allocator& allocator, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size, RendererCommand::Type command_type)
{
	RendererCommand rc;

	auto copied_resource = (RenderResourceData*)allocator.allocate(sizeof(RenderResourceData));
	memcpy(copied_resource, &resource, sizeof(RenderResourceData));
	rc.data = copied_resource;
	rc.type = command_type;
	
	switch (resource.type)
	{
		case RenderResourceData::RenderMaterial:
			copied_resource->data = allocator.allocate(sizeof(MaterialResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(MaterialResourceData));
			break;
		case RenderResourceData::Shader:
			copied_resource->data = allocator.allocate(sizeof(ShaderResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(ShaderResourceData));
			break;
		case RenderResourceData::Texture:
			copied_resource->data = allocator.allocate(sizeof(TextureResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(TextureResourceData));
			break;
		case RenderResourceData::SpriteRenderer:
			copied_resource->data = allocator.allocate(sizeof(CreateSpriteRendererData));
			memcpy(copied_resource->data, resource.data, sizeof(CreateSpriteRendererData));
			break;
		case RenderResourceData::World:
			copied_resource->data = allocator.allocate(sizeof(RenderWorldResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(RenderWorldResourceData));
			break;
		default:
			assert(!"Unknown resource data type.");
			break;
	}

	rc.dynamic_data = dynamic_data;
	rc.dynamic_data_size = dynamic_data_size;

	return rc;
}

void RenderInterface::create_resource(RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{	
	dispatch(create_or_update_resource_renderer_command(_allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::LoadResource));
}

void RenderInterface::update_resource(RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	dispatch(create_or_update_resource_renderer_command(_allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::UpdateResource));
}

void RenderInterface::deallocate_processed_commands(Allocator& allocator)
{
	_renderer.deallocate_processed_commands(allocator);
}

RenderFence& RenderInterface::create_fence()
{
	auto fence_command = create_command(RendererCommand::Fence);
	fence_command.data = _allocator.construct<RenderFence>();
	dispatch(fence_command);	
	return *(RenderFence*)fence_command.data;
}

void RenderInterface::wait_for_fence(RenderFence& fence)
{
	{
		std::unique_lock<std::mutex> lock(fence.mutex);
		fence.fence_processed.wait(lock, [&fence] { return fence.processed; });
	}
	_allocator.destroy(&fence);
}

void RenderInterface::resize(const Vector2u& resolution)
{
	ResizeData& rd = *(ResizeData*)_allocator.allocate(sizeof(ResizeData));
	rd.resolution = resolution;
	
	auto resize_command = create_command(RendererCommand::Resize);
	resize_command.data = &rd;	
	dispatch(resize_command);
}

const Vector2u& RenderInterface::resolution() const
{
	return _renderer.resolution();
}

}