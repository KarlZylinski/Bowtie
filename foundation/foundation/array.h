#pragma once

#include "collection_types.h"
#include "memory.h"

#include <memory>
#include <functional>

namespace bowtie {
	namespace array
	{
		template<typename T> void init(Array<T>& a, Allocator& allocator);
		template<typename T> Array<T> create(Allocator& allocator);
		template<typename T> void copy(Array<T>& from, Array<T>& to);
		template<typename T> void deinit(Array<T>& a);

		/// The number of elements in the array.
		template<typename T> uint32_t size(const Array<T> &a) ;
		/// Returns true if there are any elements in the array.
		template<typename T> bool any(const Array<T> &a);
		/// Returns true if the array is empty.
		template<typename T> bool empty(const Array<T> &a);
		
		/// Used to iterate over the array.
		template<typename T> T* begin(Array<T> &a);
		template<typename T> const T* begin(const Array<T> &a);
		template<typename T> T* end(Array<T> &a);
		template<typename T> const T* end(const Array<T> &a);
		
		/// Returns the first/last element of the array. Don't use these on an
		/// empty array.
		template<typename T> T& front(Array<T> &a);
		template<typename T> const T& front(const Array<T> &a);
		template<typename T> T& back(Array<T> &a);
		template<typename T> const T& back(const Array<T> &a);

		/// Changes the size of the array (does not reallocate memory unless necessary).
		template <typename T> void resize(Array<T> &a, uint32_t new_size);
		/// Removes all items in the array (does not free memory).
		template <typename T> void clear(Array<T> &a);
		/// Reallocates the array to the specified capacity.
		template<typename T> void set_capacity(Array<T> &a, uint32_t new_capacity);
		/// Makes sure that the array has at least the specified capacity.
		/// (If not, the array is grown.)
		template <typename T> void reserve(Array<T> &a, uint32_t new_capacity);
		/// Grows the array using a geometric progression formula, so that the ammortized
		/// cost of push_back() is O(1). If a min_capacity is specified, the array will
		/// grow to at least that capacity.
		template<typename T> void grow(Array<T> &a, uint32_t min_capacity = 0);
		/// Trims the array so that its capacity matches its size.
		template <typename T> void trim(Array<T> &a);

		/// Pushes the item to the end of the array.
		template<typename T> void push_back(Array<T> &a, const T &item);
		/// Pops the last item from the array. The array cannot be empty.
		template<typename T> void pop_back(Array<T> &a);
	}

	namespace array
	{
		template<typename T> inline void init(Array<T>& a, Allocator& allocator)
		{
			memset(&a, 0, sizeof(Array<T>));
			a._allocator = &allocator;
		}
		
		template<typename T> inline Array<T> create(Allocator& allocator)
		{
			Array<T> a = {0};
			a._allocator = &allocator;
			return a;
		}

		template<typename T> inline void copy(Array<T>& from, Array<T>& to)
		{
			const uint32_t n = from._size;
			array::set_capacity(to, n);
			memcpy(from._data, to._data, sizeof(T) * n);
			to._size = n;
		}

		template<typename T> inline void deinit(Array<T>& a)
		{
			a._allocator->dealloc(a._data);
		}

		template<typename T> inline uint32_t size(const Array<T> &a) 		{return a._size;}
		template<typename T> inline bool any(const Array<T> &a) 			{return a._size != 0;}
		template<typename T> inline bool empty(const Array<T> &a) 			{return a._size == 0;}
		
		template<typename T> inline T* begin(Array<T> &a) 					{return a._data;}
		template<typename T> inline const T* begin(const Array<T> &a) 		{return a._data;}
		template<typename T> inline T* end(Array<T> &a) 					{return a._data + a._size;}
		template<typename T> inline const T* end(const Array<T> &a) 		{return a._data + a._size;}
		
		template<typename T> inline T& front(Array<T> &a) 					{return a._data[0];}
		template<typename T> inline const T& front(const Array<T> &a) 		{return a._data[0];}
		template<typename T> inline T& back(Array<T> &a) 					{return a._data[a._size-1];}
		template<typename T> inline const T& back(const Array<T> &a) 		{return a._data[a._size-1];}

		template <typename T> inline void clear(Array<T> &a) {resize(a,0);}
		template <typename T> inline void trim(Array<T> &a) {set_capacity(a,a._size);}

		template <typename T> void resize(Array<T> &a, uint32_t new_size)
		{
			if (new_size > a._capacity)
				grow(a, new_size);
			a._size = new_size;
		}

		template <typename T> inline void reserve(Array<T> &a, uint32_t new_capacity)
		{
			if (new_capacity > a._capacity)
				set_capacity(a, new_capacity);
		}

		template <typename T> void remove_at(Array<T> &a, unsigned index)
		{
			if (a._size == 0)
				return;

			if (a._size != 1 && index != a._size - 1)
				a._data[index] = array::back(a);

			--a._size;
		}
				
		template <typename T, typename F> void remove(Array<T> &a, const F& predicate)
		{
			for (unsigned i = 0; i < array::size(a); ++i)
			{
				auto& element = a[i];

				if (!predicate(element))
					continue;

				remove_at(a, i);
				return;
			}
		}

		template <typename T> void remove(Array<T> &a, const T& element)
		{
			for (unsigned i = 0; i < array::size(a); ++i)
			{
				auto& element_from_list = a[i];

				if (element != element_from_list)
					continue;

				remove_at(a, i);
				return;
			}
		}

		template<typename T> void set_capacity(Array<T> &a, uint32_t new_capacity)
		{
			if (new_capacity == a._capacity)
				return;

			if (new_capacity < a._size)
				resize(a, new_capacity);

			T *new_data = 0;
			if (new_capacity > 0) {
				new_data = (T *)a._allocator->alloc(sizeof(T)*new_capacity, alignof(T));
				memcpy(new_data, a._data, sizeof(T)*a._size);
			}
			a._allocator->dealloc(a._data);
			a._data = new_data;
			a._capacity = new_capacity;
		}

		template<typename T> void grow(Array<T> &a, uint32_t min_capacity)
		{
			uint32_t new_capacity = a._capacity*2 + 8;
			if (new_capacity < min_capacity)
				new_capacity = min_capacity;
			set_capacity(a, new_capacity);
		}

		template<typename T> inline void push_back(Array<T> &a, const T &item)
		{
			if (a._size + 1 > a._capacity)
				grow(a);
			a._data[a._size++] = item;
		}

		template<typename T> inline void pop_back(Array<T> &a)
		{
			a._size--;
		}
	}

	template <typename T>
	inline T & Array<T>::operator[](uint32_t i)
	{
		return _data[i];
	}

	template <typename T>
	inline const T & Array<T>::operator[](uint32_t i) const
	{
		return _data[i];
	}
}