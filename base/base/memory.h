#pragma once

#include "memory_types.h"
#include <stdint.h>

#define TRACING 1

namespace bowtie
{

namespace memory
{
    inline void *align_forward(void* p, uint32 align);
    inline void *pointer_add(void* p, uint64 bytes);
    inline const void *pointer_add(const void* p, uint64 bytes);
    inline void *pointer_sub(void* p, uint64 bytes);
    inline const void *pointer_sub(const void* p, uint64 bytes);

    void init_allocator(Allocator* a, const char* name, CallstackCapturer* callstack_capturer);
    void deinit_allocator(Allocator* a);
}

// Aligns p to the specified alignment by moving it forward if necessary and returns the result.
inline void* memory::align_forward(void* p, uint32 align)
{
    uintptr_t pi = uintptr_t(p);
    const uint32 mod = pi % align;

    if (mod)
        pi += (align - mod);

    return (void *)pi;
}

// Returns the result of advancing p by the specified number of bytes
inline void *memory::pointer_add(void* p, uint64 bytes)
{
    return (void*)((uint8*)p + bytes);
}

inline const void *memory::pointer_add(const void* p, uint64 bytes)
{
    return (const void*)((const uint8*)p + bytes);
}

// Returns the result of moving p back by the specified number of bytes
inline void *memory::pointer_sub(void* p, uint64 bytes)
{
    return (void*)((uint8*)p - bytes);
}

inline const void *memory::pointer_sub(const void *p, uint64 bytes)
{
    return (const void*)((const uint8*)p - bytes);
}

extern PermanentMemory MainThreadMemory;
extern PermanentMemory RenderThreadMemory;

namespace memory
{
    void init(PermanentMemory* memory, void* buffer, uint32 size);
    void* alloc(PermanentMemory* memory, uint32 size, uint32 align = memory::default_align);
    void* alloc_raw(PermanentMemory* memory, uint32 size, uint32 align = memory::default_align);
    void rewind(PermanentMemory* memory);
}

namespace temp_memory
{
    void init(void* memory, uint64 total_size);
    void* alloc(uint64 size, uint32 align = memory::default_align);
    void* alloc_raw(uint64 size, uint32 align = memory::default_align);
    void new_frame();
}

namespace debug_memory
{
    void* alloc(uint32 size, uint32 align = memory::default_align);
    void* alloc_raw(uint32 size, uint32 align = memory::default_align);
    void dealloc(void* p);
}

}
