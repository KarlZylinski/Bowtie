#pragma once

namespace bowtie
{

struct Allocator;

struct Stream
{
    uint32 size;
    uint32 capacity;
    uint8* start;
    uint8* write_head;
};

namespace stream
{
    void write(Stream* s, void* data, uint32 data_size, Allocator* allocator);
}

}