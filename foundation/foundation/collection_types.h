#pragma once

#include "types.h"
#include "memory_types.h"

namespace bowtie
{
	struct VectorHeader
	{
		Allocator* allocator;
		unsigned size;
		unsigned capacity;
		unsigned element_size;
		unsigned element_alignment;
	};

	template<typename T> struct Array
	{
		T &operator[](uint32_t i);
		const T &operator[](uint32_t i) const;

		Allocator* _allocator;
		uint32_t _size;
		uint32_t _capacity;
		T* _data;
	};

	template<typename T> struct Hash
	{
	public:
		struct Entry {
			uint64_t key;
			uint32_t next;
			T value;
		};

		Array<uint32_t> _hash;
		Array<Entry> _data;
	};
}