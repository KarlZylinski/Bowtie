#pragma once

#include <foundation/matrix4.h>

#include "image.h"
#include "uniform.h"
#include "render_resource_handle.h"

namespace bowtie
{

struct RenderResourceData
{
	enum Type
	{
		RenderTarget, Shader, Texture, World, RenderMaterial, SpriteRenderer
	};

	Type type;
	void* data;
};

struct MaterialResourceData
{
	RenderResourceHandle handle;
	RenderResourceHandle shader;
	unsigned num_uniforms;
};
	
struct ShaderResourceData
{
	RenderResourceHandle handle;
	unsigned vertex_shader_source_offset;
	unsigned fragment_shader_source_offset;
};

struct TextureResourceData
{
	RenderResourceHandle handle;
	PixelFormat pixel_format;
	unsigned texture_data_size;
	unsigned texture_data_dynamic_data_offset;
	Vector2u resolution;
};

struct UniformResourceData
{
	uniform::AutomaticValue automatic_value;
	uniform::Type type;
	unsigned name_offset;
	unsigned value_offset;
};

struct CreateSpriteRendererData
{
	RenderResourceHandle world;
	unsigned num;
};

struct UpdateSpriteRendererData
{
	unsigned num;
};

struct RenderWorldResourceData
{
	RenderResourceHandle handle;
};

namespace render_resource_data
{

inline RenderResourceData create(RenderResourceData::Type type)
{
	RenderResourceData rr = { type, 0 };
	return rr;
}

} // render_resource_data

} // namespace bowtie
