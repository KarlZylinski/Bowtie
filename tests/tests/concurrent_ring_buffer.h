#pragma once

namespace bowtie
{

class Allocator;

namespace tests
{

void test_concurrent_ring_buffer(Allocator& allocator);

}

}