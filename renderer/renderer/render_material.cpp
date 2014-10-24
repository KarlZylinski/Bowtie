#include "render_material.h"
#include <foundation/array.h>
#include <cassert>

namespace bowtie
{

namespace
{

void set_uniform_value(RenderMaterial& material, Allocator& allocator, uint64_t name, const void* value, unsigned value_size)
{
	for (unsigned i = 0; i < array::size(material.uniforms); ++i)
	{
		auto& uniform = material.uniforms[i];

		if (uniform.name == name)
		{
			render_uniform::set_value(uniform, allocator, value, value_size);
			break;
		}
	}
}

}

namespace render_material
{

void init(RenderMaterial& material, Allocator& allocator, RenderResourceHandle shader)
{
	material.shader = shader;
	material.uniforms = array::create<RenderUniform>(allocator);
}

void deinit(RenderMaterial& material, Allocator& allocator)
{
	for (unsigned i = 0; i < array::size(material.uniforms); ++i)
		allocator.deallocate(material.uniforms[i].value);

	array::deinit(material.uniforms);
}

void add_uniform(RenderMaterial& material, const RenderUniform& uniform)
{
	array::push_back(material.uniforms, uniform);
}

void set_uniform_vector4_value(RenderMaterial& material, Allocator& allocator, uint64_t name, const Vector4& value)
{
	set_uniform_value(material, allocator, name, &value, sizeof(Vector4));
}

void set_uniform_unsigned_value(RenderMaterial& material, Allocator& allocator, uint64_t name, unsigned value)
{
	set_uniform_value(material, allocator, name, &value, sizeof(unsigned));
}

void set_uniform_float_value(RenderMaterial& material, Allocator& allocator, uint64_t name, float value)
{
	set_uniform_value(material, allocator, name, &value, sizeof(float));
}

} // namespace render_material

namespace render_uniform
{

void set_value(RenderUniform& uniform, Allocator& allocator, const void* value, unsigned value_size)
{
	if (uniform.value == nullptr)
		uniform.value = allocator.allocate(value_size);

	memcpy(uniform.value, value, value_size);
}

} // namespace render_uniform

} // namespace bowtie
