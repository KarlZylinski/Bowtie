#pragma once

#include <engine/render_resource_handle.h>
#include <foundation/collection_types.h>
#include "render_uniform.h"
#include <stdint.h>

namespace bowtie
{

struct RenderMaterial
{
	RenderResourceHandle shader;
	Array<RenderUniform> uniforms;
};

namespace render_material
{
	void init(RenderMaterial& material, Allocator& allocator, RenderResourceHandle shader);
	void deinit(RenderMaterial& material, Allocator& allocator);
	void add_uniform(RenderMaterial& material, const RenderUniform& uniform);
	void set_uniform_vector4_value(RenderMaterial& material, Allocator& allocator, uint64_t name, const Vector4& value);
	void set_uniform_unsigned_value(RenderMaterial& material, Allocator& allocator, uint64_t name, unsigned value);
	void set_uniform_float_value(RenderMaterial& material, Allocator& allocator, uint64_t name, float value);
}

namespace render_uniform
{
	void set_value(RenderUniform& uniform, Allocator& allocator, const void* value, unsigned value_size);
}

}