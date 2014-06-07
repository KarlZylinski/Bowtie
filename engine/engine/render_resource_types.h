#pragma once

#include <foundation/matrix4.h>

#include "image.h"
#include "resource_handle.h"

namespace bowtie
{

struct RenderResourceData
{
	enum Type
	{
		Shader, Texture, Sprite, World, Geometry
	};

	Type type;
	ResourceHandle handle;
	void* data;
};
	
struct ShaderResourceData
{
	unsigned vertex_shader_source_offset;
	unsigned fragment_shader_source_offset;
};

struct TextureResourceData
{
	image::PixelFormat pixel_format;
	unsigned texture_data_size;
	unsigned texture_data_dynamic_data_offset;
	Vector2u resolution;
};

struct SpriteResourceData
{
	ResourceHandle render_world;
	ResourceHandle texture;
	ResourceHandle shader;
	ResourceHandle geometry;
	Matrix4 model;
};

struct GeometryResourceData
{
	unsigned size;
};

} // namespace bowtie