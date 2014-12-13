#pragma once

namespace bowtie
{

struct Allocator;

struct Stream
{
	uint32 size;
	uint32 capacity;
	char* start;
	char* write_head;
};

namespace stream
{
	void write(Stream* s, void* data, uint32 data_size, Allocator* allocator);
}

}