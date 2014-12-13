#include "malloc_allocator.h"
#include "memory_types.h"
#include <cstdlib>
#include <cassert>
#include "callstack_capturer.h"
#include <cstring>
#include "allocator_helpers.h"

namespace bowtie
{

namespace internal
{

inline void* alloc(Allocator* a, uint32 size, uint32 align)
{
	const auto ts = allocator_helpers::size_with_padding(size, align);
	auto h = (Header *)memory::align_forward(malloc(ts), memory::default_align);
	auto p = allocator_helpers::data_pointer(h, align);
	allocator_helpers::fill(h, p, ts);
	++a->total_allocations;
	a->total_allocated += ts;

	#if defined(TRACING)
		auto captured_callstack = a->callstack_capturer->capture(1, p);
		allocator_helpers::add_captured_callstack(a->_captured_callstacks, &captured_callstack);
	#endif

	return p;
}

inline void dealloc(Allocator* a, void* p)
{
	if (!p)
		return;

	auto h = allocator_helpers::header(p);
	assert(a->total_allocated >= h->size);

	#if defined(TRACING)
		assert(h->tracing_marker == TRACING_MARKER);
		h->tracing_marker = 0;
	#endif

	--a->total_allocations;
	assert(a->total_allocations >= 0);
	a->total_allocated -= h->size;

	#if defined(TRACING)
		allocator_helpers::remove_captured_callstack(a->_captured_callstacks, p);
	#endif

	free(h);
}

} // namespace internal

void* MallocAllocator::alloc(uint32 size, uint32 align)
{
	auto p = internal::alloc(this, size, align);
	memset(p, 0, size);
	return p;
}

void* MallocAllocator::alloc_raw(uint32 size, uint32 align)
{
	return internal::alloc(this, size, align);
}

void MallocAllocator::dealloc(void *p)
{
	internal::dealloc(this, p);
}

} // namespace bowtie
