#pragma once

#include "collection_types.h"
#include "memory.h"

#include <memory>
#include <functional>

namespace bowtie {
    namespace vector
    {
        template<typename T> void init(Vector<T>* v, Allocator* allocator);
        template<typename T> void init(Vector<T>* v); // Will use temp memory.
        template<typename T> void copy(Vector<T>* from, Vector<T>* to);
        template<typename T> void deinit(Vector<T>* v);
        template<typename T> void resize(Vector<T>* v, uint32 new_size);
        template<typename T> void clear(Vector<T>* v);
        template<typename T> void set_capacity(Vector<T>* v, uint32 new_capacity);
        template<typename T> void reserve(Vector<T>* v, uint32 new_capacity);
        template<typename T> void grow(Vector<T>* v, uint32 min_capacity);
        template<typename T> void grow(Vector<T>* v);
        template<typename T> void trim(Vector<T>* v);
        template<typename T> void push(Vector<T>* v, const T &item);
        template<typename T> void pop(Vector<T>* v);
        template<typename T> T* last(const Vector<T>* v);
        template<typename T> void remove_at(Vector<T>* v, uint32 index);
        template<typename T, typename F> void remove(Vector<T>* v, const F& predicate);
        template<typename T> void remove(Vector<T>* v, const T& element);
    }

    namespace vector
    {
        template<typename T> inline void init(Vector<T>* v, Allocator* allocator)
        {
            memset(v, 0, sizeof(Vector<T>));
            v->allocator = allocator;
        }

        template<typename T> inline void init(Vector<T>* v)
        {
            memset(v, 0, sizeof(Vector<T>));
            v->allocator = nullptr;
        }

        template<typename T> inline void copy(Vector<T>* from, Vector<T>* to)
        {
            const uint32 n = from->size;
            set_capacity(to, n);
            memcpy(from->data, to->data, sizeof(T) * n);
            to.size = n;
        }

        template<typename T> inline void deinit(Vector<T>* v)
        {
            if (v->allocator != nullptr)
                v->allocator->dealloc(v->data);
        }
        
        template <typename T> inline void clear(Vector<T>* v)
        {
            resize(v, 0);
        }

        template <typename T> void resize(Vector<T>* v, uint32 new_size)
        {
            if (new_size > v->capacity)
                grow(v, new_size);

            v->size = new_size;
        }

        template <typename T> inline void reserve(Vector<T>* v, uint32 new_capacity)
        {
            if (new_capacity > v->capacity)
                set_capacity(v, new_capacity);
        }

        template <typename T> void remove_at(Vector<T>* v, uint32 index)
        {
            if (v->size == 0)
                return;

            if (v->size != 1 && index != v->size - 1)
                v->data[index] = *vector::last(v);

            --v->size;
        }
                
        template <typename T, typename F> void remove(Vector<T>* v, const F& predicate)
        {
            for (uint32 i = 0; i < v->size; ++i)
            {
                auto& element = a[i];

                if (!predicate(element))
                    continue;

                remove_at(v, i);
                return;
            }
        }

        template <typename T> void remove(Vector<T>* v, const T& element)
        {
            for (uint32 i = 0; i < v->size; ++i)
            {
                auto& element_from_list = a[i];

                if (element != element_from_list)
                    continue;

                remove_at(v, i);
                return;
            }
        }

        template<typename T> void set_capacity(Vector<T>* v, uint32 new_capacity)
        {
            if (new_capacity == v->capacity)
                return;

            if (new_capacity < v->size)
                resize(v, new_capacity);

            T *new_data = 0;

            if (new_capacity > 0)
            {
                if (v->allocator == nullptr)
                    new_data = (T*)temp_memory::alloc_raw(sizeof(T) * new_capacity, alignof(T));
                else    
                    new_data = (T*)v->allocator->alloc_raw(sizeof(T) * new_capacity, alignof(T));

                memcpy(new_data, v->data, sizeof(T) * v->size);
            }

            if (v->allocator != nullptr)
                v->allocator->dealloc(v->data);

            v->data = new_data;
            v->capacity = new_capacity;
        }

        template<typename T> void grow(Vector<T>* v, uint32 min_capacity)
        {
            uint32 new_capacity = v->capacity*2 + 8;

            if (new_capacity < min_capacity)
                new_capacity = min_capacity;

            set_capacity(v, new_capacity);
        }

        template<typename T> void grow(Vector<T>* v)
        {
            grow(v, 0);
        }

        template<typename T> inline void push(Vector<T>* v, const T &item)
        {
            if (v->size + 1 > v->capacity)
                grow(v);

            v->data[v->size++] = item;
        }
        
        template<typename T> T* last(const Vector<T>* v)
        {
            return v->data + (v->size - 1);
        }

        template<typename T> inline void pop(Vector<T>* v)
        {
            --v->size;
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