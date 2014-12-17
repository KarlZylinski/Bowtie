#include "memory.h"
#include <cstring>
#include <cstdlib>
#include "allocator_helpers.h"
#include <mutex>

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
    Assert(a->total_allocations == 0, "Allocator not empty on deinit");
    Assert(a->total_allocated == 0, "Allocator not empty on deinit");
}

}

namespace memory
{

void init(PermanentMemory* memory, void* buffer, uint32 size)
{
    Assert(size > 0, "Memory block passet to memory::init is null");
    Assert(buffer != nullptr, "Memory block passed to memory::init is null");
    memory->start = (uint8*)buffer;
    memory->end = memory->start + size;
    memory->head = memory->start;
}

void* alloc(PermanentMemory* memory, uint32 size, uint32 align)
{
    auto p = alloc_raw(memory, size, align);
    memset(p, 0, size);
    return p;
}

void* alloc_raw(PermanentMemory* memory, uint32 size, uint32 align)
{
    auto total_size = size + align;
    Assert(memory->head + total_size <= memory->end, "Out of memory in permanent allocator");
    auto p = memory::align_forward(memory->head, memory::default_align);
    memory->head += total_size;
    return p;
}

void rewind(PermanentMemory* memory)
{
    memory->head = memory->start;
}

}

namespace temp_memory
{

std::mutex mutex;
uint8* start;
uint8* end;
uint8* head;
uint8* head_at_frame_start;

void init(void* memory, uint64 total_size)
{
    start = (uint8*)memory;
    end = start + total_size;
    head = start;
    head_at_frame_start = head;
    memset(head, 0xffffffff, total_size);
}

void* alloc(uint64 size, uint32 align)
{
    auto p = alloc_raw(size, align);
    memset(p, 0, size);
    return p;
}

void* alloc_raw(uint64 size, uint32 align)
{   
    std::lock_guard<std::mutex> lock(mutex);
    auto total_size = size + align;

    if (head + total_size > end)
    {
        Assert(head_at_frame_start < head + total_size, "Allocation will go past where head was at frame start");
        head = start;
    }
    
    Assert(head_at_frame_start <= head || head_at_frame_start > head + total_size, "Allocation will go past where head was at frame start");
    auto p = memory::align_forward(head, memory::default_align);
    head += total_size;
    return p;
}

void new_frame()
{
    head_at_frame_start = head;
}

}

namespace debug_memory
{

void* alloc(uint32 size, uint32 align)
{
    auto p = alloc_raw(size, align);
    memset(p, 0, size);
    return p;
}

void* alloc_raw(uint32 size, uint32 align)
{
    #ifndef _DEBUG
        Assert(!"Trying to use debug allocation in non-debug mode.");
    #endif

    auto total_size = size + align;
    return memory::align_forward(malloc(total_size), memory::default_align);
}

void dealloc(void* p)
{
    #ifndef _DEBUG
        Assert(!"Trying to use debug allocation in non-debug mode.");
    #endif

    free(p);
}

}
}