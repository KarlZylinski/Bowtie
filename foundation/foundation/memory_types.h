#pragma once

#include "callstack_capturer_types.h"

namespace bowtie
{

namespace memory
{
	static const unsigned default_align = 8;
	static const unsigned max_callstacks = 4096;
}

struct CallstackCapturer;

struct Allocator
{
	CallstackCapturer* callstack_capturer;
	CapturedCallstack _captured_callstacks[memory::max_callstacks];
	char* name;
	unsigned total_allocations;
	unsigned total_allocated;

	virtual void* alloc(unsigned size, unsigned align = memory::default_align) = 0;
	virtual void* alloc_raw(unsigned size, unsigned align = memory::default_align) = 0; // Does not memset to zero.
	virtual void dealloc(void* p) = 0;
};

}