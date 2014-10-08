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
		Drawable, Geometry, RenderTarget, Shader, Texture, World, RenderMaterial
	};

	Type type;
	RenderResourceHandle handle;
	void* data;
};

struct MaterialResourceData
{
	RenderResourceHandle shader;
	unsigned num_uniforms;
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
	RenderResourceHandle render_world;
	RenderResourceHandle texture;
	RenderResourceHandle material;
	RenderResourceHandle geometry;
	unsigned num_vertices;
	Matrix4 model;
	int depth;
};

struct UniformResourceData
{
	uniform::AutomaticValue automatic_value;
	char* name;
	uniform::Type type;
	void* value;
};

struct GeometryResourceData
{
	unsigned size;
};

} // namespace bowtie