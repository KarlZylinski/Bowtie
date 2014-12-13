#include "allocator_helpers.h"
#include "memory.h"
#include "callstack_capturer.h"
#include <cassert>

namespace bowtie
{

namespace allocator_helpers
{

void add_captured_callstack(CapturedCallstack* callstacks, const CapturedCallstack& cc)
{
	for (unsigned i = 0; i < memory::max_callstacks; ++i)
	{
		if (!callstacks[i].used)
		{
			callstacks[i] = cc;
			return;
		}
	}

	assert(!"Out of callstacks. Increase max_callstacks in memory.cpp.");
}

void remove_captured_callstack(CapturedCallstack* callstacks, void* p)
{
	for (unsigned i = 0; i < memory::max_callstacks; ++i) {
		if (callstacks[i].ptr == p)
		{
			callstacks[i].used = false;
			return;
		}
	}

	assert(!"Failed to find callstack in remove_captured_callstack.");
}

void ensure_captured_callstacks_unused(CallstackCapturer* callstack_capturer, CapturedCallstack* callstacks)
{
	for (unsigned i = 0; i < memory::max_callstacks; ++i)
	{
		if (!callstacks[i].used)
			continue;

		callstack_capturer->print_callstack("Memory leak stack trace", callstacks + i);
	}
}

}

}