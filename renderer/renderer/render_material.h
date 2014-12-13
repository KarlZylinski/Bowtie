#pragma once
#include <engine/render_resource_handle.h>
#include "render_uniform.h"

namespace bowtie
{

struct RenderMaterial
{
	RenderResourceHandle shader;
	uint32 num_uniforms;
	RenderUniform* uniforms;
};

namespace render_material
{
	void init(RenderMaterial* material, Allocator* allocator, uint32 num_uniforms, RenderResourceHandle shader);
	void deinit(RenderMaterial* material, Allocator* allocator);
	void set_uniform_vector4_value(RenderMaterial* material, Allocator* allocator, uint64 name, const Vector4* value);
	void set_uniform_uint32_value(RenderMaterial* material, Allocator* allocator, uint64 name, uint32 value);
	void set_uniform_float_value(RenderMaterial* material, Allocator* allocator, uint64 name, float value);
}

}