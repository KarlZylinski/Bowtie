#include "memory.h"
#include <cstring>
#include <cstdlib>
#include "allocator_helpers.h"
#include <cassert>

namespace bowtie
{

namespace memory
{

void init_allocator(Allocator* a, const char* name, CallstackCapturer* callstack_capturer)
{
	a->name = (char*)malloc(strlen(name) + 1);
	strcpy(a->name, name);
	a->callstack_capturer = callstack_capturer;
	a->total_allocated = 0;
	a->total_allocations = 0;
	memset(a->_captured_callstacks, 0, sizeof(CapturedCallstack) * memory::max_callstacks);
}

void deinit_allocator(Allocator* a)
{
	#ifdef TRACING
		allocator_helpers::ensure_captured_callstacks_unused(a->callstack_capturer, a->_captured_callstacks);
	#endif

	free(a->name);
	assert(a->total_allocations == 0);
	assert(a->total_allocated == 0);
}

}

}