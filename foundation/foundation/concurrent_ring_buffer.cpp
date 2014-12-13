#include "concurrent_ring_buffer.h"
#include "memory.h"
#include <cassert>

namespace bowtie
{

namespace internal
{

void write(char** buffer, const void* data, unsigned size)
{
	memcpy(*buffer, data, size);
	*buffer += size;
}

unsigned used(ConcurrentRingBuffer* b)
{
	if (b->write_head <= b->consume_head && b->has_wrapped)
	{
		auto end = (char*)memory::pointer_add(b->start, b->size * b->element_size);
		auto used = (unsigned)(end - b->consume_head) + (unsigned)(b->write_head - b->start);
		return used;
	}

	auto used = (unsigned)(b->write_head - b->consume_head);
	return used;
}

} // namespace internal

namespace concurrent_ring_buffer
{

void init(ConcurrentRingBuffer* b, Allocator* allocator, unsigned size, unsigned element_size)
{
	b->size = size;
	b->element_size = element_size;
	b->start = (char*)allocator->alloc_raw(element_size * size);
	b->write_head = b->start;
	b->has_wrapped = false;
	b->consume_head = b->start;
	b->allocator = allocator;
}

void deinit(ConcurrentRingBuffer* b)
{
	b->allocator->dealloc(b->start);
}

void write_one(ConcurrentRingBuffer* b, const void* data)
{
	std::lock_guard<std::mutex> lock(b->mutex);
	auto end = (char*)memory::pointer_add(b->start, b->size * b->element_size);
	internal::write(&b->write_head, data, b->element_size);
	
	if (b->write_head == end)
	{
		b->write_head = b->start;
		b->has_wrapped = true;
	}
}

void* peek(ConcurrentRingBuffer* b)
{
	std::lock_guard<std::mutex> lock(b->mutex);

	if (internal::used(b) == 0)
		return nullptr;

	return b->consume_head;
}

void consume_one(ConcurrentRingBuffer* b)
{
	std::lock_guard<std::mutex> lock(b->mutex);
	auto end = (char*)memory::pointer_add(b->start, b->size * b->element_size);
	memset(b->consume_head, 0, b->element_size);
	b->consume_head += b->element_size;

	if (b->consume_head == end)
	{
		b->consume_head = b->start;
		b->has_wrapped = false;
	}
}

bool fits_one(ConcurrentRingBuffer* b)
{
	std::lock_guard<std::mutex> lock(b->mutex);
	return (b->size * b->element_size) - internal::used(b) >= b->element_size;
}

} // namespace concurrent_ring_buffer

} // namespace bowtie
