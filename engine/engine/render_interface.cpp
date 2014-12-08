#include "render_interface.h"

#include <cassert>
#include <condition_variable>
#include <mutex>

#include <foundation/memory.h>

#include "image.h"
#include "material.h"
#include "render_fence.h"
#include "resource_manager.h"
#include "texture.h"
#include "world.h"
#include <foundation/concurrent_ring_buffer.h>

namespace bowtie
{

namespace internal
{

RendererCommand create_or_update_resource_renderer_command(Allocator& allocator, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size, RendererCommand::Type command_type)
{
	RendererCommand rc;

	auto copied_resource = (RenderResourceData*)allocator.alloc_raw(sizeof(RenderResourceData));
	memcpy(copied_resource, &resource, sizeof(RenderResourceData));
	rc.data = copied_resource;
	rc.type = command_type;

	switch (resource.type)
	{
	case RenderResourceData::RenderMaterial:
		copied_resource->data = allocator.alloc_raw(sizeof(MaterialResourceData));
		memcpy(copied_resource->data, resource.data, sizeof(MaterialResourceData));
		break;
	case RenderResourceData::Shader:
		copied_resource->data = allocator.alloc_raw(sizeof(ShaderResourceData));
		memcpy(copied_resource->data, resource.data, sizeof(ShaderResourceData));
		break;
	case RenderResourceData::Texture:
		copied_resource->data = allocator.alloc_raw(sizeof(TextureResourceData));
		memcpy(copied_resource->data, resource.data, sizeof(TextureResourceData));
		break;
	case RenderResourceData::SpriteRenderer:
		copied_resource->data = allocator.alloc_raw(sizeof(CreateSpriteRendererData));
		memcpy(copied_resource->data, resource.data, sizeof(CreateSpriteRendererData));
		break;
	case RenderResourceData::World:
		copied_resource->data = allocator.alloc_raw(sizeof(RenderWorldResourceData));
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

RendererCommand create_command(Allocator& allocator, RendererCommand::Type type)
{
	RendererCommand command;
	memset(&command, 0, sizeof(RendererCommand));
	command.type = type;

	switch (type)
	{
	case RendererCommand::SetUniformValue:
		command.data = allocator.alloc(sizeof(SetUniformValueData));
		break;
	}

	return command;
}

void dispatch(RenderInterface& ri, const RendererCommand& command)
{
	if (!concurrent_ring_buffer::fits_one(*ri._unprocessed_commands))
		render_interface::wait_until_idle(ri);
	
	concurrent_ring_buffer::write_one(*ri._unprocessed_commands, &command);

	{
		std::lock_guard<std::mutex> unprocessed_commands_exists_lock(*ri._unprocessed_commands_exist_mutex);
		*ri._unprocessed_commands_exist = true;
	}

	ri._wait_for_unprocessed_commands_to_exist->notify_all();
}

void create_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	dispatch(ri, create_or_update_resource_renderer_command(*ri.allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::LoadResource));
}

void update_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	dispatch(ri, create_or_update_resource_renderer_command(*ri.allocator, resource, dynamic_data, dynamic_data_size, RendererCommand::UpdateResource));
}

RenderResourceHandle create_handle(Array<RenderResourceHandle>& free_handles)
{
	assert(array::any(free_handles) && "Out of render resource handles!");
	RenderResourceHandle handle = array::back(free_handles);
	array::pop_back(free_handles);
	return handle;
}

} // namespace internal

namespace render_interface
{

void init(RenderInterface& ri, Allocator& allocator, ConcurrentRingBuffer& unprocessed_commands, bool& unprocessed_commands_exist,
		  std::mutex& unprocessed_commands_exist_mutex, std::condition_variable& wait_for_unprocessed_commands_to_exist)
{
	ri.allocator = &allocator;
	ri._unprocessed_commands = &unprocessed_commands;
	ri._unprocessed_commands_exist = &unprocessed_commands_exist;
	ri._unprocessed_commands_exist_mutex = &unprocessed_commands_exist_mutex;
	ri._wait_for_unprocessed_commands_to_exist = &wait_for_unprocessed_commands_to_exist;
	array::init(ri._free_handles, allocator);
	auto num_handles = render_resource_handle::num;
	array::set_capacity(ri._free_handles, num_handles);

	for (unsigned handle = num_handles; handle > 0; --handle)
		array::push_back(ri._free_handles, RenderResourceHandle(handle));
}

void deinit(RenderInterface& ri)
{
	array::deinit(ri._free_handles);
}

RenderResourceHandle create_handle(RenderInterface& ri)
{
	return internal::create_handle(ri._free_handles);
}

void free_handle(RenderInterface& ri, RenderResourceHandle handle)
{
	assert(handle < render_resource_handle::num && "Trying to free render resource handle with a higher value than render_resource_handle::num.");
	array::push_back(ri._free_handles, handle);
}

void create_texture(RenderInterface& ri, Texture& texture)
{
	assert(texture.render_handle == RenderResourceHandle::NotInitialized && "Trying to create already created texture");

	Image& image = *texture.image;

	auto texture_resource = render_resource_data::create(RenderResourceData::Texture);

	auto trd = TextureResourceData();
	trd.handle = internal::create_handle(ri._free_handles);
	trd.resolution = image.resolution;
	trd.texture_data_dynamic_data_offset = 0;
	trd.texture_data_size = image.data_size;
	trd.pixel_format = image.pixel_format;

	texture_resource.data = &trd;
	internal::create_resource(ri, texture_resource, image.data, image.data_size);
	texture.render_handle = trd.handle;
}

void create_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	internal::create_resource(ri, resource, dynamic_data, dynamic_data_size);
}

void update_resource(RenderInterface& ri, RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	internal::update_resource(ri, resource, dynamic_data, dynamic_data_size);
}

void create_resource(RenderInterface& ri, RenderResourceData& resource)
{
	internal::create_resource(ri, resource, nullptr, 0);
}

void update_resource(RenderInterface& ri, RenderResourceData& resource)
{
	internal::update_resource(ri, resource, nullptr, 0);
}

void create_render_world(RenderInterface& ri, World& world)
{
	assert(world.render_handle == RenderResourceHandle::NotInitialized);
	auto render_world_data = render_resource_data::create(RenderResourceData::World);
	RenderWorldResourceData rwrd;
	rwrd.handle = internal::create_handle(ri._free_handles);
	render_world_data.data = &rwrd;
	world.render_handle = rwrd.handle;
	internal::create_resource(ri, render_world_data, nullptr, 0);
}

RendererCommand create_command(RenderInterface& ri, RendererCommand::Type type)
{
	return internal::create_command(*ri.allocator, type);
}

void dispatch(RenderInterface& ri, const RendererCommand& command)
{
	internal::dispatch(ri, command);
}

void dispatch(RenderInterface& ri, const RendererCommand& command, void* dynamic_data, unsigned dynamic_data_size)
{
	auto command_with_dynamic_data = command;
	command_with_dynamic_data.dynamic_data = ri.allocator->alloc_raw(dynamic_data_size);
	command_with_dynamic_data.dynamic_data_size = dynamic_data_size;
	memcpy(command_with_dynamic_data.dynamic_data, dynamic_data, dynamic_data_size);
	internal::dispatch(ri, command_with_dynamic_data);
}

RenderFence& create_fence(RenderInterface& ri)
{
	auto fence_command = internal::create_command(*ri.allocator, RendererCommand::Fence);
	fence_command.data = new(ri.allocator->alloc_raw(sizeof(RenderFence), alignof(RenderFence))) RenderFence(); 
	internal::dispatch(ri, fence_command);
	return *(RenderFence*)fence_command.data;
}

void wait_for_fence(RenderInterface& ri, RenderFence& fence)
{
	{
		std::unique_lock<std::mutex> lock(fence.mutex);
		fence.fence_processed.wait(lock, [&fence] { return fence.processed; });
	}

	ri.allocator->dealloc(&fence);
}

void wait_until_idle(RenderInterface& ri)
{
	wait_for_fence(ri, create_fence(ri));
}

void resize(RenderInterface& ri, const Vector2u& resolution)
{
	ResizeData& rd = *(ResizeData*)ri.allocator->alloc(sizeof(ResizeData));
	rd.resolution = resolution;
	auto resize_command = internal::create_command(*ri.allocator, RendererCommand::Resize);
	resize_command.data = &rd;
	dispatch(ri, resize_command);
}

}
}
