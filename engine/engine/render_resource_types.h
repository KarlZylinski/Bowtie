#pragma once

#include <foundation/matrix4.h>

#include "resource_handle.h"

namespace bowtie
{

struct RenderResourceData
{
	enum Type
	{
		Shader, Texture, Sprite
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
	unsigned texture_data_size;
	unsigned texture_data_dynamic_data_offset;
	unsigned width, height;
};

struct SpriteResourceData
{
	ResourceHandle image;
	Matrix4 model;
};

} // namespace bowtie