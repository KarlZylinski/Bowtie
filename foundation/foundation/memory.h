#pragma once

#include "memory_types.h"
#include <stdint.h>

#define TRACING 1

namespace bowtie
{

namespace memory
{
	inline void *align_forward(void *p, uint32 align);
	inline void *pointer_add(void *p, uint32 bytes);
	inline const void *pointer_add(const void *p, uint32 bytes);
	inline void *pointer_sub(void *p, uint32 bytes);
	inline const void *pointer_sub(const void *p, uint32 bytes);

	void init_allocator(Allocator* a, const char* name, CallstackCapturer* callstack_capturer);
	void deinit_allocator(Allocator* a);
}

// Aligns p to the specified alignment by moving it forward if necessary and returns the result.
inline void *memory::align_forward(void *p, uint32 align)
{
	uintptr_t pi = uintptr_t(p);
	const uint32 mod = pi % align;
	if (mod)
		pi += (align - mod);
	return (void *)pi;
}

// Returns the result of advancing p by the specified number of bytes
inline void *memory::pointer_add(void *p, uint32 bytes)
{
	return (void*)((char *)p + bytes);
}

inline const void *memory::pointer_add(const void *p, uint32 bytes)
{
	return (const void*)((const char *)p + bytes);
}

// Returns the result of moving p back by the specified number of bytes
inline void *memory::pointer_sub(void *p, uint32 bytes)
{
	return (void*)((char *)p - bytes);
}

inline const void *memory::pointer_sub(const void *p, uint32 bytes)
{
	return (const void*)((const char *)p - bytes);
}

}
