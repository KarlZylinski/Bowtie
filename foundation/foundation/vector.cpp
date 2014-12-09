#include "vector.h"
#include <cstring>
#include "memory.h"

namespace bowtie
{

namespace internal
{

void* grow(VectorHeader* header, void* vector)
{
	auto new_capacity = header->capacity == 0 ? 1 : header->capacity * 2;
	auto new_data = header->allocator->alloc_raw(new_capacity * header->element_size, header->element_alignment);
	memcpy(new_data, vector, header->size * header->element_size);
	header->allocator->dealloc(vector);
	header->capacity = new_capacity;
	return new_data;
}

}

namespace vector
{

void* init(VectorHeader* header, Allocator* allocator, unsigned element_size, unsigned element_alignment)
{
	memset(header, 0, sizeof(VectorHeader));
	header->allocator = allocator;
	header->element_alignment = element_alignment;
	header->element_size = element_size;
	return internal::grow(header, nullptr);
}

void clear(VectorHeader* header)
{
	header->size = 0;
}

void* push(VectorHeader* header, void* vector, const void* item)
{
	void* v = header->size == header->capacity ? internal::grow(header, vector) : vector;
	memcpy(memory::pointer_add(v, header->size * header->element_size), item, header->element_size);
	++header->size;
	return v;
}

}

}
