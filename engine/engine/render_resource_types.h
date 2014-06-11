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
		Shader, Texture, Drawable, World, Geometry, Target
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

struct DrawableResourceData
{
	ResourceHandle render_world;
	ResourceHandle texture;
	ResourceHandle shader;
	ResourceHandle geometry;
	unsigned num_vertices;
	Matrix4 model;
};

struct GeometryResourceData
{
	unsigned size;
};

} // namespace bowtie