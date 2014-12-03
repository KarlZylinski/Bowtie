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

namespace
{

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

RendererCommand create_command_internal(Allocator& allocator, RendererCommand::Type type)
{
	RendererCommand command;
	memset(&command, 0, sizeof(RendererCommand));
	command.type = type;

	switch (type)
	{
	case RendererCommand::SetUniformValue:
		command.data = allocator.allocate(sizeof(SetUniformValueData));
		break;
	}

	return command;
}

void dispatch_internal(IRenderer& renderer, const RendererCommand& command)
{
	renderer.add_renderer_command(command);
}

void create_resource_internal(IRenderer& renderer, Allocator& allocator, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	dispatch_internal(renderer, create_or_update_resource_renderer_command(allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::LoadResource));
}

void update_resource_internal(IRenderer& renderer, Allocator& allocator, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	dispatch_internal(renderer, create_or_update_resource_renderer_command(allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::UpdateResource));
}

} // anonymous namespace

namespace render_interface
{

void init(RenderInterface& ri, Allocator& allocator, IRenderer& renderer)
{
	ri.allocator = &allocator;
	ri.renderer = &renderer;
}

void create_texture(RenderInterface& ri, Texture& texture)
{
	assert(texture.render_handle == RenderResourceHandle::NotInitialized && "Trying to create already created texture");

	Image& image = *texture.image;

	auto texture_resource = render_resource_data::create(RenderResourceData::Texture);

	auto trd = TextureResourceData();
	trd.handle = ri.renderer->create_handle();
	trd.resolution = image.resolution;
	trd.texture_data_dynamic_data_offset = 0;
	trd.texture_data_size = image.data_size;
	trd.pixel_format = image.pixel_format;

	texture_resource.data = &trd;
	create_resource_internal(*ri.renderer, *ri.allocator, texture_resource, image.data, image.data_size);
	texture.render_handle = trd.handle;
}

void create_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	create_resource_internal(*ri.renderer, *ri.allocator, resource, dynamic_data, dynamic_data_size);
}

void update_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	update_resource_internal(*ri.renderer, *ri.allocator, resource, dynamic_data, dynamic_data_size);
}

void create_resource(RenderInterface& ri, RenderResourceData& resource)
{
	create_resource_internal(*ri.renderer, *ri.allocator, resource, nullptr, 0);
}

void update_resource(RenderInterface& ri, RenderResourceData& resource)
{
	update_resource_internal(*ri.renderer, *ri.allocator, resource, nullptr, 0);
}

void create_render_world(RenderInterface& ri, World& world)
{
	assert(world.render_handle == RenderResourceHandle::NotInitialized);
	auto render_world_data = render_resource_data::create(RenderResourceData::World);
	RenderWorldResourceData rwrd;
	rwrd.handle = ri.renderer->create_handle();
	render_world_data.data = &rwrd;
	world.render_handle = rwrd.handle;
	create_resource_internal(*ri.renderer, *ri.allocator, render_world_data, nullptr, 0);
}

RendererCommand create_command(RenderInterface& ri, RendererCommand::Type type)
{
	return create_command_internal(*ri.allocator, type);
}

void dispatch(RenderInterface& ri, const RendererCommand& command)
{
	dispatch_internal(*ri.renderer, command);
}

void dispatch(RenderInterface& ri, const RendererCommand& command, void* dynamic_data, unsigned dynamic_data_size)
{
	auto command_with_dynamic_data = command;
	command_with_dynamic_data.dynamic_data = ri.allocator->allocate(dynamic_data_size);
	command_with_dynamic_data.dynamic_data_size = dynamic_data_size;
	memcpy(command_with_dynamic_data.dynamic_data, dynamic_data, dynamic_data_size);
	dispatch_internal(*ri.renderer, command_with_dynamic_data);
}

RenderFence& create_fence(RenderInterface& ri)
{
	auto fence_command = create_command_internal(*ri.allocator, RendererCommand::Fence);
	fence_command.data = ri.allocator->construct<RenderFence>();
	dispatch_internal(*ri.renderer, fence_command);
	return *(RenderFence*)fence_command.data;
}

void wait_for_fence(RenderInterface& ri, RenderFence& fence)
{
	{
		std::unique_lock<std::mutex> lock(fence.mutex);
		fence.fence_processed.wait(lock, [&fence] { return fence.processed; });
	}

	ri.allocator->destroy(&fence);
}

void wait_until_idle(RenderInterface& ri)
{
	wait_for_fence(ri, create_fence(ri));
}

void resize(RenderInterface& ri, const Vector2u& resolution)
{
	ResizeData& rd = *(ResizeData*)ri.allocator->allocate(sizeof(ResizeData));
	rd.resolution = resolution;
	auto resize_command = create_command_internal(*ri.allocator, RendererCommand::Resize);
	resize_command.data = &rd;
	dispatch(ri, resize_command);
}

}
}
