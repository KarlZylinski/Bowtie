#pragma once

#include "memory.h"

namespace bowtie
{

struct CapturedCallstack;
struct CallstackCapturer;

const uint32 HEADER_PAD_VALUE = 0xffffffffu;

#if defined(TRACING)
const uint32 TRACING_MARKER = 0xfffffffeu;
#endif

struct Header
{
    #if defined(TRACING)
        uint32 tracing_marker;
    #endif
    uint64 size;
};

namespace allocator_helpers
{

void add_captured_callstack(CapturedCallstack* callstacks, const CapturedCallstack* cc);
void remove_captured_callstack(CapturedCallstack* callstacks, void* p);
void ensure_captured_callstacks_unused(CallstackCapturer* callstack_capturer, CapturedCallstack* callstacks);
inline void* data_pointer(Header *header, uint32 align);
inline Header *header(void *data);
inline void fill(Header *header, void *data, uint32 size);
inline uint32 size_with_padding(uint32 size, uint32 align);

#include "allocator_helpers.inl"

}

}
