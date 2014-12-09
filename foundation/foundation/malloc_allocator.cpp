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

const unsigned HEADER_PAD_VALUE = 0xffffffffu;

#if defined(TRACING)
	const unsigned TRACING_MARKER = 0xfffffffeu;
#endif

// Header stored at the beginning of a memory allocation to indicate the
// size of the allocated data.
struct Header {
	#if defined(TRACING)
		unsigned tracing_marker;
	#endif
	unsigned size;
};

// Given a pointer to the header, returns a pointer to the data that follows it.
inline void *data_pointer(Header *header, unsigned align) {
	auto p = header + 1;
	return memory::align_forward(p, align);
}

// Given a pointer to the data, returns a pointer to the header before it.
inline Header *header(void *data)
{
	auto p = (unsigned *)data;
	while (p[-1] == HEADER_PAD_VALUE)
		--p;
	return (Header *)memory::pointer_sub(p, sizeof(Header));
}

// Stores the size in the header and pads with HEADER_PAD_VALUE up to the
// data pointer.
inline void fill(Header *header, void *data, unsigned size)
{
	header->size = size;
	#if defined(TRACING)
		header->tracing_marker = TRACING_MARKER;
	#endif

	auto p = (unsigned *)memory::pointer_add(header, sizeof(Header));
	while (p < data)
		*p++ = HEADER_PAD_VALUE;
}

inline unsigned size_with_padding(unsigned size, unsigned align) {
	return size + align * 2 + sizeof(Header);
}

inline void* alloc(Allocator& a, unsigned size, unsigned align)
{
	const unsigned ts = size_with_padding(size, align);
	auto h = (Header *)memory::align_forward(malloc(ts), memory::default_align);
	void *p = data_pointer(h, align);
	fill(h, p, ts);
	++a.total_allocations;
	a.total_allocated += ts;

	#if defined(TRACING)
		allocator_helpers::add_captured_callstack(a._captured_callstacks, a.callstack_capturer->capture(1, p));
	#endif

	return p;
}

inline void dealloc(Allocator& a, void* p)
{
	if (!p)
		return;

	auto h = header(p);
	assert(a.total_allocated >= h->size);

	#if defined(TRACING)
		assert(h->tracing_marker == TRACING_MARKER);
		h->tracing_marker = 0;
	#endif

	--a.total_allocations;
	assert(a.total_allocations >= 0);
	a.total_allocated -= h->size;

	#if defined(TRACING)
		allocator_helpers::remove_captured_callstack(a._captured_callstacks, p);
	#endif

	free(h);
}

} // namespace internal

void* MallocAllocator::alloc(unsigned size, unsigned align)
{
	auto p = internal::alloc(*this, size, align);
	memset(p, 0, size);
	return p;
}

void* MallocAllocator::alloc_raw(unsigned size, unsigned align)
{
	return internal::alloc(*this, size, align);
}

void MallocAllocator::dealloc(void *p)
{
	internal::dealloc(*this, p);
}

} // namespace bowtie
