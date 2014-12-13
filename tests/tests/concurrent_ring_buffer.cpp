#include "concurrent_ring_buffer.h"
#include <foundation/concurrent_ring_buffer.h>
#include <foundation/memory.h>
#include <thread>
#include <cassert>

namespace bowtie
{

namespace tests
{

struct Haze
{
	int lax;
	short bulgur;
};

void writer(ConcurrentRingBuffer* b)
{
	auto i = 0;
	Haze h1 = { 1, 3 };
	Haze h2 = { 1, 5 };
	Haze h3 = { 1, 7 };

	while (i < 200000)
	{
		if (!concurrent_ring_buffer::fits_one(b))
			continue;

		auto r = rand() % 3;

		auto h = r == 0
			? h1
			: r == 1
			? h2
			: h3;

		concurrent_ring_buffer::write_one(b, &h);

		++i;
	}
}

void consumer(ConcurrentRingBuffer* b)
{
	auto i = 0;

	while (i < 200000)
	{
		auto h = (Haze*)concurrent_ring_buffer::peek(b);

		if (h != nullptr)
		{
			assert(h->lax == 1);
			assert(h->bulgur == 3 || h->bulgur == 5 || h->bulgur == 7);
			concurrent_ring_buffer::consume_one(b);
			++i;
		}
	}
}

void test_concurrent_ring_buffer(Allocator* allocator)
{
	ConcurrentRingBuffer b;
	concurrent_ring_buffer::init(&b, allocator, 8, sizeof(Haze));
	std::thread w(&writer, &b);
	std::thread c(&consumer, &b);
	w.join();
	c.join();
	concurrent_ring_buffer::deinit(&b);
}

}

}