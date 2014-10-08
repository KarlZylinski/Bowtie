#include "render_uniform_utils.h"
#include "render_uniform.h"
#include <foundation/memory.h>
#include <foundation/vector4.h>
#include <cassert>

namespace bowtie
{

namespace render_uniform
{

namespace
{
	
void set_value(RenderUniform& uniform, Allocator& allocator, const Vector4& value)
{
	if (uniform.value == nullptr)
		uniform.value = allocator.allocate(sizeof(Vector4));

	memcpy(uniform.value, &value, sizeof(Vector4));
}

void set_value(RenderUniform& uniform, Allocator& allocator, unsigned value)
{
	if (uniform.value == nullptr)
		uniform.value = allocator.allocate(sizeof(unsigned));

	memcpy(uniform.value, &value, sizeof(unsigned));
}

void set_value(RenderUniform& uniform, Allocator& allocator, float value)
{
	if (uniform.value == nullptr)
		uniform.value = allocator.allocate(sizeof(float));

	memcpy(uniform.value, &value, sizeof(float));
}

}

void set_value(RenderUniform& uniform, Allocator& allocator, void* value)
{
	switch (uniform.type)
	{
	case uniform::Float:
		set_value(uniform, allocator, *(float*)value);
		break;
	case uniform::Texture1:
	case uniform::Texture2:
	case uniform::Texture3:
		set_value(uniform, allocator, *(unsigned*)value);
		break;
	case uniform::Vec4:
		set_value(uniform, allocator, *(Vector4*)value);
		break;
	default:
		assert(!"Unknown uniform type");
		break;
	}
}

};

};
