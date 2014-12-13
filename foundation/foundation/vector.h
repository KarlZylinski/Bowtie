#pragma once

#include "collection_types.h"
#include "memory.h"

#include <memory>
#include <functional>

namespace bowtie {
	namespace vector
	{
		template<typename T> void init(Vector<T>* a, Allocator* allocator);
		template<typename T> void copy(Vector<T>* from, Vector<T>* to);
		template<typename T> void deinit(Vector<T>* a);
		template<typename T> void resize(Vector<T>* a, uint32 new_size);
		template<typename T> void clear(Vector<T>* a);
		template<typename T> void set_capacity(Vector<T>* a, uint32 new_capacity);
		template<typename T> void reserve(Vector<T>* a, uint32 new_capacity);
		template<typename T> void grow(Vector<T>* a, uint32 min_capacity);
		template<typename T> void grow(Vector<T>* a);
		template<typename T> void trim(Vector<T>* a);
		template<typename T> void push(Vector<T>* a, const T &item);
		template<typename T> void pop(Vector<T>* a);
		template<typename T> T* last(const Vector<T>* a);
		template<typename T> void remove_at(Vector<T>* a, uint32 index);
		template<typename T, typename F> void remove(Vector<T>* a, const F& predicate);
		template<typename T> void remove(Vector<T>* a, const T& element);
	}

	namespace vector
	{
		template<typename T> inline void init(Vector<T>* a, Allocator* allocator)
		{
			memset(a, 0, sizeof(Vector<T>));
			a->allocator = allocator;
		}

		template<typename T> inline void copy(Vector<T>* from, Vector<T>* to)
		{
			const uint32 n = from->size;
			set_capacity(to, n);
			memcpy(from->data, to->data, sizeof(T) * n);
			to.size = n;
		}

		template<typename T> inline void deinit(Vector<T>* a)
		{
			a->allocator->dealloc(a->data);
		}
		
		template <typename T> inline void clear(Vector<T>* a)
		{
			resize(a, 0);
		}

		template <typename T> void resize(Vector<T>* a, uint32 new_size)
		{
			if (new_size > a->capacity)
				grow(a, new_size);

			a->size = new_size;
		}

		template <typename T> inline void reserve(Vector<T>* a, uint32 new_capacity)
		{
			if (new_capacity > a->capacity)
				set_capacity(a, new_capacity);
		}

		template <typename T> void remove_at(Vector<T>* a, uint32 index)
		{
			if (a->size == 0)
				return;

			if (a->size != 1 && index != a->size - 1)
				a->data[index] = *vector::last(a);

			--a->size;
		}
				
		template <typename T, typename F> void remove(Vector<T>* a, const F& predicate)
		{
			for (uint32 i = 0; i < a->size; ++i)
			{
				auto& element = a[i];

				if (!predicate(element))
					continue;

				remove_at(a, i);
				return;
			}
		}

		template <typename T> void remove(Vector<T>* a, const T& element)
		{
			for (uint32 i = 0; i < a->size; ++i)
			{
				auto& element_from_list = a[i];

				if (element != element_from_list)
					continue;

				remove_at(a, i);
				return;
			}
		}

		template<typename T> void set_capacity(Vector<T>* a, uint32 new_capacity)
		{
			if (new_capacity == a->capacity)
				return;

			if (new_capacity < a->size)
				resize(a, new_capacity);

			T *new_data = 0;

			if (new_capacity > 0) {
				new_data = (T*)a->allocator->alloc(sizeof(T) * new_capacity, alignof(T));
				memcpy(new_data, a->data, sizeof(T) * a->size);
			}

			a->allocator->dealloc(a->data);
			a->data = new_data;
			a->capacity = new_capacity;
		}

		template<typename T> void grow(Vector<T>* a, uint32 min_capacity)
		{
			uint32 new_capacity = a->capacity*2 + 8;

			if (new_capacity < min_capacity)
				new_capacity = min_capacity;

			set_capacity(a, new_capacity);
		}

		template<typename T> void grow(Vector<T>* a)
		{
			grow(a, 0);
		}

		template<typename T> inline void push(Vector<T>* a, const T &item)
		{
			if (a->size + 1 > a->capacity)
				grow(a);

			a->data[a->size++] = item;
		}
		
		template<typename T> T* last(const Vector<T>* a)
		{
			return a->data + (a->size - 1);
		}

		template<typename T> inline void pop(Vector<T>* a)
		{
			--a->size;
		}
	}

	template <typename T>
	inline T& Vector<T>::operator[](uint32 i)
	{
		return data[i];
	}

	template <typename T>
	inline const T& Vector<T>::operator[](uint32 i) const
	{
		return data[i];
	}
}