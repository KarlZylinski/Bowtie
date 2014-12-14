#include "render_uniform.h"
#include <base/memory.h>
#include <cstring>

namespace bowtie
{

namespace render_uniform
{
    void set_value(RenderUniform* uniform, Allocator* allocator, const void* value, uint32 value_size)
    {
        if (uniform->value == nullptr)
            uniform->value = allocator->alloc(value_size);

        memcpy(uniform->value, value, value_size);
    }

} // namespace render_uniform

}