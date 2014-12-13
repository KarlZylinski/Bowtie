#pragma once

#include "memory_types.h"
#include "types.h"

namespace bowtie
{
template<typename T> struct Vector
{
	Allocator* allocator;
	uint32 size;
	uint32 capacity;
	T* data;
	T &operator[](uint32 i);
	const T &operator[](uint32 i) const;
};

template<typename T> struct Hash
{
	struct Entry {
		uint64 key;
		uint32 next;
		T value;
	};

	Vector<uint32> _hash;
	Vector<Entry> _data;
};
}