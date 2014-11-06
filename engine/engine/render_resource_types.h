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
		Drawable, Geometry, RenderTarget, Shader, Texture, World, RenderMaterial, RectangleRenderer
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

struct DrawableResourceData
{
	RenderResourceHandle handle;
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
	uniform::Type type;
	unsigned name_offset;
	unsigned value_offset;
};

struct GeometryResourceData
{
	RenderResourceHandle handle;
	unsigned size;
};

struct CreateRectangleRendererData
{
	RenderResourceHandle world;
	unsigned num;
};

struct UpdateRectangleRendererData
{
	unsigned num;
};

struct RenderWorldResourceData
{
	RenderResourceHandle handle;
};

} // namespace bowtie
