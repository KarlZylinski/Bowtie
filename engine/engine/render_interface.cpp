#include "render_interface.h"

#include <cassert>
#include <condition_variable>
#include <mutex>

#include <foundation/memory.h>

#include "render_fence.h"
#include "renderer.h"
#include "sprite.h"
#include "image.h"
#include "world.h"
#include "texture.h"
#include "resource_manager.h"

namespace bowtie
{

RenderInterface::RenderInterface(Renderer& renderer, Allocator& allocator) : _allocator(allocator), _renderer(renderer)
{
}

void RenderInterface::create_texture(Texture& texture)
{
	assert(texture.render_handle.type == RenderResourceHandle::NotInitialized && "Trying to create already created texture");

	Image& image = *texture.image;

	auto texture_resource = create_render_resource_data(RenderResourceData::Texture);

	auto trd = TextureResourceData();
	trd.resolution = image.resolution;
	trd.texture_data_dynamic_data_offset = 0;
	trd.texture_data_size = image.data_size;
	trd.pixel_format = image.pixel_format;

	texture_resource.data = &trd;

	create_resource(texture_resource, image.data, image.data_size);

	texture.render_handle = texture_resource.handle;
}

ResourceHandle get_shader_or_default(ResourceManager& resource_manager, Sprite& sprite)
{
	auto shader = sprite.shader();

	if (shader.type != ResourceHandle::NotInitialized)
		return shader;

	return resource_manager.get_default(ResourceManager::RT_Shader);
}

void RenderInterface::spawn_sprite(World& world, Sprite& sprite, ResourceManager& resource_manager)
{
	assert(sprite.render_handle().type == RenderResourceHandle::NotInitialized && "Trying to spawn already spawned sprite");
	assert(sprite.texture() != nullptr);

	auto sprite_rrd = create_render_resource_data(RenderResourceData::Sprite);

	SpriteResourceData sprite_resource_data;
	sprite_resource_data.texture = sprite.texture()->render_handle;
	sprite_resource_data.render_world = world.render_handle();
	sprite_resource_data.model = sprite.model_matrix();
	sprite_resource_data.shader = get_shader_or_default(resource_manager, sprite);

	auto geometry = sprite.geometry();

	if (geometry.type == ResourceHandle::NotInitialized)
	{
		auto geometry_rrd = create_render_resource_data(RenderResourceData::Geometry);
		GeometryResourceData geometry_data;
		geometry_data.size = sizeof(float) * 5 * 6;
		geometry_rrd.data = &geometry_data;
		auto geometry_dynamic_data = (float*)_allocator.allocate(geometry_data.size);
		memcpy(geometry_dynamic_data, sprite.geometry_data(), geometry_data.size);
		create_resource(geometry_rrd, (void*)geometry_dynamic_data, geometry_data.size);
		sprite_resource_data.geometry = geometry_rrd.handle;
		sprite.set_geometry(geometry_rrd.handle);
	}
	else
		sprite_resource_data.geometry = geometry;

	sprite_rrd.data = &sprite_resource_data;

	create_resource(sprite_rrd);
	sprite.set_render_handle(sprite_rrd.handle);
}

void RenderInterface::create_render_world(World& world)
{
	assert(world.render_handle().type == RenderResourceHandle::NotInitialized);

	auto render_world_data = create_render_resource_data(RenderResourceData::World);
	world.set_render_handle(render_world_data.handle);
	create_resource(render_world_data);
}

RenderResourceData RenderInterface::create_render_resource_data(RenderResourceData::Type type)
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

bool RenderInterface::is_setup() const
{
	return _renderer.is_setup();
}

bool RenderInterface::active() const
{
	return _renderer.active();
}

void RenderInterface::dispatch(const RendererCommand& command)
{
	_renderer.add_renderer_command(command);
}

void RenderInterface::create_resource(RenderResourceData& resource, void* dynamic_data, unsigned dynamic_data_size)
{
	assert(resource.handle.type != ResourceHandle::NotInitialized && "Trying to load an uninitialized resource");

	RendererCommand rc;

	auto copied_resource = (RenderResourceData*)_allocator.allocate(sizeof(RenderResourceData));
	memcpy(copied_resource, &resource, sizeof(RenderResourceData));
	rc.data = copied_resource;

	rc.type = RendererCommand::LoadResource;
	
	switch (resource.type)
	{
		case RenderResourceData::Shader:
			copied_resource->data = _allocator.allocate(sizeof(ShaderResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(ShaderResourceData));
			break;
		case RenderResourceData::Texture:
			copied_resource->data = _allocator.allocate(sizeof(TextureResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(TextureResourceData));
			break;
		case RenderResourceData::Sprite:
			copied_resource->data = _allocator.allocate(sizeof(SpriteResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(SpriteResourceData));			
			break;
		case RenderResourceData::Geometry:
			copied_resource->data = _allocator.allocate(sizeof(GeometryResourceData));
			memcpy(copied_resource->data, resource.data, sizeof(GeometryResourceData));
			break;
		case RenderResourceData::World:
			// No data
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