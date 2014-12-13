#pragma once

#include "memory_types.h"
#include "types.h"

namespace bowtie
{
template<typename T> struct Vector
{
	Allocator* allocator;
	unsigned size;
	unsigned capacity;
	T* data;
	T &operator[](uint32_t i);
	const T &operator[](uint32_t i) const;
};

template<typename T> struct Hash
{
	struct Entry {
		uint64 key;
		uint32 next;
		T value;
	};

	Vector<unsigned> _hash;
	Vector<Entry> _data;
};
}