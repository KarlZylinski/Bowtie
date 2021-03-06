#include "allocator_helpers.h"
#include "memory.h"
#include "callstack_capturer.h"

namespace bowtie
{

namespace allocator_helpers
{

void add_captured_callstack(CapturedCallstack* callstacks, const CapturedCallstack* cc)
{
    for (uint32 i = 0; i < memory::max_callstacks; ++i)
    {
        if (!callstacks[i].used)
        {
            callstacks[i] = *cc;
            return;
        }
    }

    Error("Out of callstacks. Increase max_callstacks in memory.cpp.");
}

void remove_captured_callstack(CapturedCallstack* callstacks, void* p)
{
    for (uint32 i = 0; i < memory::max_callstacks; ++i) {
        if (callstacks[i].ptr == p)
        {
            callstacks[i].used = false;
            return;
        }
    }

    Error("Failed to find callstack in remove_captured_callstack.");
}

void ensure_captured_callstacks_unused(CallstackCapturer* callstack_capturer, CapturedCallstack* callstacks)
{
    for (uint32 i = 0; i < memory::max_callstacks; ++i)
    {
        if (!callstacks[i].used)
            continue;

        callstack_capturer->print_callstack(L"Memory leak stack trace", callstacks + i);
    }
}

}

}