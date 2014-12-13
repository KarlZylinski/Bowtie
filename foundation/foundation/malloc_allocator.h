#pragma once

#include "memory.h"

namespace bowtie
{

struct MallocAllocator : Allocator
{
	void* alloc(unsigned size, unsigned align = memory::default_align);
	void* alloc_raw(unsigned size, unsigned align = memory::default_align);
	void dealloc(void* p);
};

}
