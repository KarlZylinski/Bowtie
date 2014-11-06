#pragma once

namespace bowtie
{

class Allocator;

struct Stream
{
	unsigned size;
	unsigned capacity;
	char* start;
	char* write_head;
};

namespace stream
{
	void write(Stream& s, void* data, unsigned data_size, Allocator& allocator);
}

}