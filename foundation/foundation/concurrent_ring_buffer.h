#pragma once

#include <mutex>

namespace bowtie
{

class Allocator;

struct ConcurrentRingBuffer
{
	unsigned size;
	char* start;
	char* write_head;
	char* consume_head;
	std::mutex mutex;
	Allocator* allocator; 
};

namespace concurrent_ring_buffer
{
	void init(ConcurrentRingBuffer& b, Allocator& allocator, unsigned size);
	void deinit(ConcurrentRingBuffer& b);
	void write(ConcurrentRingBuffer& b, const void* data, unsigned size);
	char* consume(ConcurrentRingBuffer& b, Allocator& allocator, unsigned size);
	char* consume_all(ConcurrentRingBuffer& b, Allocator& allocator);
	bool will_fit(ConcurrentRingBuffer& b, unsigned size);
}

}