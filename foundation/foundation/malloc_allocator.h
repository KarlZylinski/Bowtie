#pragma once

#include "memory.h"

namespace bowtie
{

struct MallocAllocator : Allocator
{
	void* alloc(uint32 size, uint32 align = memory::default_align);
	void* alloc_raw(uint32 size, uint32 align = memory::default_align);
	void dealloc(void* p);
};

}
