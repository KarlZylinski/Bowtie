#pragma once

#include "stream.h"
#include "memory_types.h"
#include "memory.h"
#include <cstring>

namespace bowtie
{

namespace
{

void grow(Stream* s, uint32 minimum_size)
{
    auto new_capacity = (s->capacity == 0 ? 1 : s->capacity * 2) + minimum_size;
    auto new_data = (uint8*)temp_memory::alloc_raw(new_capacity);
    memcpy(new_data, s->start, s->size);
    auto write_head_offset = uint32(s->write_head - s->start);
    s->capacity = new_capacity;
    s->start = new_data;
    s->write_head = (uint8*)memory::pointer_add(s->start, write_head_offset);
}

}

namespace stream
{

void write(Stream* s, void* data, uint32 data_size)
{
    if (s->size + data_size > s->capacity)
        grow(s, s->size + data_size);

    memcpy(s->write_head, data, data_size);
    s->write_head = (uint8*)memory::pointer_add(s->write_head, data_size);
    s->size += data_size;
}

}

}