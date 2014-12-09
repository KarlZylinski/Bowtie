#include "vector.h"
#include <foundation/vector.h>

namespace bowtie
{

namespace tests
{

void test_vector(Allocator* allocator)
{
	VectorHeader header;
	auto numbers = vector::init(&header, allocator, sizeof(unsigned), alignof(unsigned));
	unsigned n1 = 5;
	unsigned n2 = 2;
	unsigned n3 = 1;
	numbers = vector::push(&header, numbers, &n1);
	numbers = vector::push(&header, numbers, &n2);
	numbers = vector::push(&header, numbers, &n3);
	numbers = vector::push(&header, numbers, &n1);
	numbers = vector::push(&header, numbers, &n2);
	numbers = vector::push(&header, numbers, &n3);
	numbers = vector::push(&header, numbers, &n1);
	numbers = vector::push(&header, numbers, &n2);
	numbers = vector::push(&header, numbers, &n3);
	numbers = vector::push(&header, numbers, &n1);
	numbers = vector::push(&header, numbers, &n2);
	numbers = vector::push(&header, numbers, &n3);
	allocator->dealloc(numbers);
}

}

}