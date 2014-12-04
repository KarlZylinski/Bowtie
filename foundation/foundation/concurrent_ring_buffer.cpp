#include "concurrent_ring_buffer.h"
#include "memory.h"
#include <cassert>

namespace bowtie
{

namespace internal
{

void write(char** buffer, void* data, unsigned size)
{
	memcpy(*buffer, data, size);
	*buffer += size;
}

unsigned used(ConcurrentRingBuffer& b)
{
	if (b.write_head < b.consume_head)
	{
		auto end = (char*)memory::pointer_add(b.start, b.size);
		auto used = (unsigned)(end - b.consume_head) + (unsigned)(b.write_head - b.start);
		return used;
	}

	auto used = (unsigned)(b.write_head - b.consume_head);
	return used;
}

} // namespace internal

namespace concurrent_ring_buffer
{

void init(ConcurrentRingBuffer& b, Allocator& allocator, unsigned size)
{
	b.size = size;
	b.start = (char*)allocator.allocate(size);
	b.write_head = b.start;
	b.consume_head = b.start;
	b.allocator = &allocator;
}

void deinit(ConcurrentRingBuffer& b)
{
	b.allocator->deallocate(b.start);
}

void write(ConcurrentRingBuffer& b, const void* data, unsigned size)
{
	std::lock_guard<std::mutex> lock(b.mutex);
	auto end = (char*)memory::pointer_add(b.start, b.size);
	auto data_read_head = (char*)data;

	if (memory::pointer_add(b.write_head, size) > end)
	{
		auto remaining_to_end = (unsigned)(end - b.write_head);
		internal::write(&b.write_head, data_read_head, remaining_to_end);
		data_read_head += remaining_to_end;
		size -= remaining_to_end;
		b.write_head = b.start;
	}

	internal::write(&b.write_head, data_read_head, size);
}

void* consume(ConcurrentRingBuffer& b, Allocator& allocator, unsigned size)
{
	auto consumed = (char*)allocator.allocate(size);
	auto consumed_write_head = consumed;
	std::lock_guard<std::mutex> lock(b.mutex);
	auto end = (char*)memory::pointer_add(b.start, b.size);

	if (memory::pointer_add(b.consume_head, size) > end)
	{
		auto remaining_to_end = (unsigned)(end - b.consume_head);
		internal::write(&consumed_write_head, b.consume_head, remaining_to_end);
		memset(b.consume_head, 0, remaining_to_end);
		size -= remaining_to_end;
		b.consume_head = b.start;
	}

	internal::write(&consumed_write_head, b.consume_head, size);
	memset(b.consume_head, 0, size);
	b.consume_head += size;
	return consumed;
}

ConsumedRingBufferData consume_all(ConcurrentRingBuffer& b, Allocator& allocator)
{
	unsigned size = internal::used(b);
	ConsumedRingBufferData consumed;
	consumed.data = consume(b, allocator, size);
	consumed.size = size;
	return consumed;
}

bool will_fit(ConcurrentRingBuffer& b, unsigned size)
{
	std::lock_guard<std::mutex> lock(b.mutex);
	return b.size - internal::used(b) >= size;
}

} // namespace concurrent_ring_buffer

} // namespace bowtie
