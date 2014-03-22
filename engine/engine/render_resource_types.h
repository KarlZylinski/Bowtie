#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct RenderResourceData
{
	enum Type
	{
		Shader
	};

	Type type;
	RenderResourceHandle handle;
	void* data;
};
	
struct ShaderResourceData
{
	unsigned vertex_shader_source_offset;
	unsigned fragment_shader_source_offset;
};

} // namespace bowtie