#pragma once

#include <engine/uniform.h>
#include <foundation/vector4.h>
#include <stdint.h>

namespace bowtie
{

struct RenderUniform
{
	RenderUniform(uniform::Type type, uint64_t name, unsigned location, uniform::AutomaticValue automatic_value);
	RenderUniform(uniform::Type type, uint64_t name, unsigned location);

	uniform::AutomaticValue automatic_value;
	uint64_t name;
	unsigned location;
	uniform::Type type;
	void* value;
};

}