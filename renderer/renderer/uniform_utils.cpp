#include "uniform_utils.h"
#include "uniform.h"
#include <foundation/memory.h>
#include <foundation/vector4.h>

namespace bowtie
{

namespace uniform
{

void SetValue(Uniform& uniform, Allocator& allocator, const Vector4& value)
{
	allocator.deallocate(uniform.value);
	uniform.value = allocator.allocate(sizeof(Vector4));
	memcpy(uniform.value, &value, sizeof(Vector4));
}

void SetValue(Uniform& uniform, Allocator& allocator, unsigned value)
{
	allocator.deallocate(uniform.value);
	uniform.value = allocator.allocate(sizeof(unsigned));
	memcpy(uniform.value, &value, sizeof(unsigned));
}

void SetValue(Uniform& uniform, Allocator& allocator, float value)
{
	allocator.deallocate(uniform.value);
	uniform.value = allocator.allocate(sizeof(float));
	memcpy(uniform.value, &value, sizeof(float));
}

};

};
