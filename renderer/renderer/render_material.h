#pragma once

#include <engine/render_resource_handle.h>
#include "render_uniform.h"
#include <stdint.h>

namespace bowtie
{

struct RenderMaterial
{
	RenderResourceHandle shader;
	unsigned num_uniforms;
	RenderUniform* uniforms;
};

namespace render_material
{
	void init(RenderMaterial& material, Allocator& allocator, unsigned num_uniforms, RenderResourceHandle shader);
	void deinit(RenderMaterial& material, Allocator& allocator);
	void set_uniform_vector4_value(RenderMaterial& material, Allocator& allocator, uint64_t name, const Vector4& value);
	void set_uniform_unsigned_value(RenderMaterial& material, Allocator& allocator, uint64_t name, unsigned value);
	void set_uniform_float_value(RenderMaterial& material, Allocator& allocator, uint64_t name, float value);
}

}