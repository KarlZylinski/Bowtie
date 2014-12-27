#include "concurrent_ring_buffer.h"
#include "memory.h"

namespace bowtie
{

namespace internal
{

void write(uint8** buffer, const void* data, uint32 size)
{
    memcpy(*buffer, data, size);
    *buffer += size;
}

uint32 used(ConcurrentRingBuffer* b)
{
    if (b->write_head <= b->consume_head && b->has_wrapped)
    {
        auto end = (uint8*)memory::pointer_add(b->start, b->size * b->element_size);
        auto used = (uint32)(end - b->consume_head) + (uint32)(b->write_head - b->start);
        return used;
    }

    auto used = (uint32)(b->write_head - b->consume_head);
    return used;
}

} // namespace internal

namespace concurrent_ring_buffer
{

void init(ConcurrentRingBuffer* b, Allocator* , uint32 size, uint32 element_size)
{
    b->size = size;
    b->element_size = element_size;
    b->start = (uint8*)memory::alloc_raw(&RenderThreadMemory, element_size * size);// (uint8*)allocator->alloc_raw(element_size * size);
    b->write_head = b->start;
    b->has_wrapped = false;
    b->consume_head = b->start;
    //b->allocator = allocator;
}

void deinit(ConcurrentRingBuffer*)
{
    //b->allocator->dealloc(b->start);
}

void write_one(ConcurrentRingBuffer* b, const void* data)
{
    std::lock_guard<std::mutex> lock(b->mutex);
    auto end = (uint8*)memory::pointer_add(b->start, b->size * b->element_size);
    internal::write(&b->write_head, data, b->element_size);
    
    if (b->write_head == end)
    {
        b->write_head = b->start;
        b->has_wrapped = true;
    }
}

void* peek(ConcurrentRingBuffer* b)
{
    std::lock_guard<std::mutex> lock(b->mutex);

    if (internal::used(b) == 0)
        return nullptr;

    return b->consume_head;
}

void consume_one(ConcurrentRingBuffer* b)
{
    std::lock_guard<std::mutex> lock(b->mutex);
    auto end = (uint8*)memory::pointer_add(b->start, b->size * b->element_size);
    memset(b->consume_head, 0, b->element_size);
    b->consume_head += b->element_size;

    if (b->consume_head == end)
    {
        b->consume_head = b->start;
        b->has_wrapped = false;
    }
}

bool fits_one(ConcurrentRingBuffer* b)
{
    std::lock_guard<std::mutex> lock(b->mutex);
    return (b->size * b->element_size) - internal::used(b) >= b->element_size;
}

} // namespace concurrent_ring_buffer

} // namespace bowtie
