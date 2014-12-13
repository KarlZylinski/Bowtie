#pragma once

#include <engine/uniform.h>
#include <foundation/vector4.h>
#include <stdint.h>

namespace bowtie
{

struct Allocator;
struct RenderUniform
{
	uniform::AutomaticValue automatic_value;
	uint64_t name;
	unsigned location;
	uniform::Type type;
	void* value;
};

namespace render_uniform
{
	void set_value(RenderUniform* uniform, Allocator* allocator, const void* value, unsigned value_size);
}

}