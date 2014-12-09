#pragma once

#include "types.h"
#include "memory_types.h"
#include <cassert>

/// All collection types assume that they are used to store POD objects. I.e. they:
///
/// * Don't call constructors and destructors on elements.
/// * Move elements with memmove().
///
/// If you want to store items that are not PODs, use something other than these collection
/// classes.
namespace bowtie
{
	/// Dynamically resizable array of POD objects.
	template<typename T> struct Array
	{
		T &operator[](uint32_t i);
		const T &operator[](uint32_t i) const;

		Allocator* _allocator;
		uint32_t _size;
		uint32_t _capacity;
		T* _data;
	};

	/// Hash from an uint64_t to POD objects. If you want to use a generic key
	/// object, use a hash function to map that object to an uint64_t.
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

	/*namespace array
	{
		inline unsigned remove(void* arr, unsigned element_size, unsigned size, unsigned index)
		{
			assert(size != 0 && "Trying to remove from empty array");
			assert(index < size && "Tried to remove outside of array");

			if (index == size - 1)
				return size - 1;

			memcpy(memory::pointer_add(arr, element_size * index), memory::pointer_add(arr, element_size * size - 1), element_size);
			return size - 1;
		}
	}*/
}