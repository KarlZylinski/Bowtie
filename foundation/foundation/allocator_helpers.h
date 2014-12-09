#pragma once

#include "memory.h"

namespace bowtie
{

struct CapturedCallstack;
struct CallstackCapturer;

const unsigned HEADER_PAD_VALUE = 0xffffffffu;

#if defined(TRACING)
const unsigned TRACING_MARKER = 0xfffffffeu;
#endif

struct Header
{
	#if defined(TRACING)
		unsigned tracing_marker;
	#endif
	unsigned size;
};

namespace allocator_helpers
{

void add_captured_callstack(CapturedCallstack* callstacks, const CapturedCallstack& cc);
void remove_captured_callstack(CapturedCallstack* callstacks, void* p);
void ensure_captured_callstacks_unused(CallstackCapturer* callstack_capturer, CapturedCallstack* callstacks);
inline void* data_pointer(Header *header, unsigned align);
inline Header *header(void *data);
inline void fill(Header *header, void *data, unsigned size);
inline unsigned size_with_padding(unsigned size, unsigned align);

#include "allocator_helpers.inl"

}

}
