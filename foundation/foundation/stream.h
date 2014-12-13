#pragma once

namespace bowtie
{

struct Allocator;

struct Stream
{
	unsigned size;
	unsigned capacity;
	char* start;
	char* write_head;
};

namespace stream
{
	void write(Stream* s, void* data, unsigned data_size, Allocator* allocator);
}

}