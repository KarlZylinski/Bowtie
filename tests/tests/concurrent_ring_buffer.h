#pragma once

namespace bowtie
{

struct Allocator;

namespace tests
{

void test_concurrent_ring_buffer(Allocator& allocator);

}

}