#pragma once
#include <engine/uniform.h>
#include <base/vector4.h>

namespace bowtie
{

struct Allocator;
struct RenderUniform
{
    uniform::AutomaticValue automatic_value;
    uint64 name;
    uint32 location;
    uniform::Type type;
    uint8 value[4];
};

namespace render_uniform
{
    void set_value(RenderUniform* uniform, const void* value, uint32 value_size);
}

}