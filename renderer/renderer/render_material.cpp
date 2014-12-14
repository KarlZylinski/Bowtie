#include "render_material.h"
#include <base/memory.h>
#include <cassert>

namespace bowtie
{

namespace internal
{

void set_uniform_value(RenderMaterial* m, uint64 name, const void* value, uint32 value_size)
{
    for (uint32 i = 0; i < m->num_uniforms; ++i)
    {
        auto uniform = m->uniforms + i;

        if (uniform->name == name)
        {
            render_uniform::set_value(uniform, value, value_size);
            break;
        }
    }
}

}

namespace render_material
{

void init(RenderMaterial* m, uint32 num_uniforms, RenderResourceHandle shader)
{
    m->shader = shader;
    m->num_uniforms = num_uniforms;
}

void set_uniform_vector4_value(RenderMaterial* m, uint64 name, const Vector4* value)
{
    internal::set_uniform_value(m, name, value, sizeof(Vector4));
}

void set_uniform_uint32_value(RenderMaterial* m, uint64 name, uint32 value)
{
    internal::set_uniform_value(m, name, &value, sizeof(uint32));
}

void set_uniform_real32_value(RenderMaterial* m, uint64 name, real32 value)
{
    internal::set_uniform_value(m, name, &value, sizeof(real32));
}

} // namespace render_material

} // namespace bowtie
