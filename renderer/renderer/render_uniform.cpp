#include "render_uniform.h"
#include <foundation/memory.h>

namespace bowtie
{

RenderUniform::RenderUniform(uniform::Type type, uint64_t name, unsigned location, uniform::AutomaticValue automatic_value) :
	automatic_value(automatic_value), name(name), location(location), type(type), value(nullptr)
{}


RenderUniform::RenderUniform(uniform::Type type, uint64_t name, unsigned location) :
	automatic_value(uniform::None), name(name), location(location), type(type), value(nullptr)
{}

namespace render_uniform
{
	void set_value(RenderUniform& uniform, Allocator& allocator, const void* value, unsigned value_size)
	{
		if (uniform.value == nullptr)
			uniform.value = allocator.allocate(value_size);

		memcpy(uniform.value, value, value_size);
	}

} // namespace render_uniform

}