#pragma once

#include <mutex>
#include <foundation/option.h>

namespace bowtie
{

struct Allocator;

struct ConcurrentRingBuffer
{
	unsigned size;
	unsigned element_size;
	char* start;
	char* write_head;
	bool has_wrapped;
	char* consume_head;
	std::mutex mutex;
	Allocator* allocator; 
};

struct ConsumedRingBufferData
{
	void* data;
	unsigned size;
};

namespace concurrent_ring_buffer
{
	void init(ConcurrentRingBuffer& b, Allocator& allocator, unsigned size, unsigned element_size);
	void deinit(ConcurrentRingBuffer& b);
	void write_one(ConcurrentRingBuffer& b, const void* data);
	void* peek(ConcurrentRingBuffer& b);
	void consume_one(ConcurrentRingBuffer& b);
	bool fits_one(ConcurrentRingBuffer& b);
}

}