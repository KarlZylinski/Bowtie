#pragma once
#include <mutex>
#include <base/option.h>

namespace bowtie
{

struct Allocator;

struct ConcurrentRingBuffer
{
    uint32 size;
    uint32 element_size;
    uint8* start;
    uint8* write_head;
    bool has_wrapped;
    uint8* consume_head;
    std::mutex mutex;
    Allocator* allocator; 
};

struct ConsumedRingBufferData
{
    void* data;
    uint32 size;
};

namespace concurrent_ring_buffer
{
    void init(ConcurrentRingBuffer* b, Allocator* allocator, uint32 size, uint32 element_size);
    void deinit(ConcurrentRingBuffer* b);
    void write_one(ConcurrentRingBuffer* b, const void* data);
    void* peek(ConcurrentRingBuffer* b);
    void consume_one(ConcurrentRingBuffer* b);
    bool fits_one(ConcurrentRingBuffer* b);
}

}