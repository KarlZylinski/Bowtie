#pragma once

#include "callstack_capturer_types.h"

namespace bowtie
{

namespace memory
{
    static const uint32 default_align = 8;
    static const uint32 max_callstacks = 4096;
}

struct PermanentMemory
{
    uint8* start;
    uint8* end;
    uint8* head;
};

struct CallstackCapturer;

struct Allocator
{
    CallstackCapturer* callstack_capturer;
    CapturedCallstack _captured_callstacks[memory::max_callstacks];
    char* name;
    uint32 total_allocations;
    uint64 total_allocated;

    virtual void* alloc(uint64 size, uint32 align = memory::default_align) = 0;
    virtual void* alloc_raw(uint64 size, uint32 align = memory::default_align) = 0; // Does not memset to zero.
    virtual void dealloc(void* p) = 0;
};

}