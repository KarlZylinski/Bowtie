#include "render_material.h"
#include <foundation/memory.h>
#include <cassert>

namespace bowtie
{

namespace internal
{

void set_uniform_value(RenderMaterial* m, Allocator* allocator, uint64_t name, const void* value, unsigned value_size)
{
	for (unsigned i = 0; i < m->num_uniforms; ++i)
	{
		auto uniform = m->uniforms + i;

		if (uniform->name == name)
		{
			render_uniform::set_value(uniform, allocator, value, value_size);
			break;
		}
	}
}

}

namespace render_material
{

void init(RenderMaterial* m, Allocator* allocator, unsigned num_uniforms, RenderResourceHandle shader)
{
	m->shader = shader;
	m->num_uniforms = num_uniforms;
	m->uniforms = (RenderUniform*)allocator->alloc(num_uniforms * sizeof(RenderUniform));
}

void deinit(RenderMaterial* m, Allocator* allocator)
{
	for (unsigned i = 0; i < m->num_uniforms; ++i)
		allocator->dealloc(m->uniforms[i].value);

	allocator->dealloc(m->uniforms);
}

void set_uniform_vector4_value(RenderMaterial* m, Allocator* allocator, uint64_t name, const Vector4* value)
{
	internal::set_uniform_value(m, allocator, name, value, sizeof(Vector4));
}

void set_uniform_unsigned_value(RenderMaterial* m, Allocator* allocator, uint64_t name, unsigned value)
{
	internal::set_uniform_value(m, allocator, name, &value, sizeof(unsigned));
}

void set_uniform_float_value(RenderMaterial* m, Allocator* allocator, uint64_t name, float value)
{
	internal::set_uniform_value(m, allocator, name, &value, sizeof(float));
}

} // namespace render_material

} // namespace bowtie
