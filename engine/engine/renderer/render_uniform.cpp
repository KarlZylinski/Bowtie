#include "render_uniform.h"
#include <base/memory.h>
#include <cstring>

namespace bowtie
{

namespace render_uniform
{
    void set_value(RenderUniform* uniform, const void* value, uint32 value_size)
    {
        memcpy(uniform->value, value, value_size);
    }

} // namespace render_uniform

}