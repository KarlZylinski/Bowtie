#pragma once

#include "vector.h"
#include "collection_types.h"
#include "option.h"

namespace bowtie {
	namespace hash
	{
		template<typename T> void init(Hash<T>* a, Allocator* allocator);
		template<typename T> void copy(Hash<T>* from, Hash<T>* to);
		template<typename T> void deinit(Hash<T>* a);
		template<typename T> bool has(const Hash<T>* h, uint64 key);
		template<typename T> const T& get(const Hash<T>* h, uint64 key, const T& default);
		template<typename T> void set(Hash<T>* h, uint64 key, const T* value);
		template<typename T> void remove(Hash<T>* h, uint64 key);
		template<typename T> void reserve(Hash<T>* h, uint32 size);
		template<typename T> void clear(Hash<T>* h);
		template<typename T> const typename Hash<T>::Entry* begin(const Hash<T>* h);
		template<typename T> const typename Hash<T>::Entry* end(const Hash<T>* h);
	}

	namespace multi_hash
	{
		template<typename T> const typename Hash<T>::Entry* find_first(const Hash<T>* h, uint64 key);
		template<typename T> const typename Hash<T>::Entry* find_next(const Hash<T>* h, const typename Hash<T>::Entry* e);
		template<typename T> uint32 count(const Hash<T>* h, uint64 key);
		template<typename T> void get(const Hash<T>* h, uint64 key, Vector<T> *items);
		template<typename T> void insert(Hash<T>* h, uint64 key, const T* value);
		template<typename T> void remove(Hash<T>* h, const typename Hash<T>::Entry* e);
		template<typename T> void remove_all(Hash<T>* h, uint64 key);
	}

	namespace hash_internal
	{
		const uint32 END_OF_LIST = 0xffffffffu;
		
		struct FindResult
		{
			uint32 hash_i;
			uint32 data_prev;
			uint32 data_i;
		};	

		template<typename T> uint32 add_entry(Hash<T>* h, uint64 key)
		{
			typename Hash<T>::Entry e;
			e.key = key;
			e.next = END_OF_LIST;
			uint32 ei = h->_data.size;
			vector::push(&h->_data, e);
			return ei;
		}

		template<typename T> void erase(Hash<T>* h, const FindResult* fr)
		{
			if (fr->data_prev == END_OF_LIST)
				h->_hash[fr->hash_i] = h->_data[fr->data_i].next;
			else
				h->_data[fr->data_prev].next = h->_data[fr->data_i].next;

			if (fr->data_i == h->_data.size - 1) {
				vector::pop(&h->_data);
				return;
			}

			h->_data[fr->data_i] = h->_data[h->_data.size - 1];
			FindResult last = find(h, h->_data[fr->data_i].key);

			if (last.data_prev != END_OF_LIST)
				h->_data[last.data_prev].next = fr->data_i;
			else
				h->_hash[last.hash_i] = fr->data_i;
		}

		template<typename T> FindResult find(const Hash<T>* h, uint64 key)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (h->_hash.size == 0)
				return fr;

			fr.hash_i = key % h->_hash.size;
			fr.data_i = h->_hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (h->_data[fr.data_i].key == key)
					return fr;
				fr.data_prev = fr.data_i;
				fr.data_i = h->_data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> FindResult find(const Hash<T>* h, const typename Hash<T>::Entry* e)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (h->_hash.size == 0)
				return fr;

			fr.hash_i = e->key % vector::size(h->_hash);
			fr.data_i = h->_hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (h->_data + fr.data_i == e)
					return fr;

				fr.data_prev = fr.data_i;
				fr.data_i = h->_data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> uint32 find_or_fail(const Hash<T>* h, uint64 key)
		{
			return find(h, key).data_i;
		}

		template<typename T> uint32 find_or_make(Hash<T>* h, uint64 key)
		{
			const FindResult fr = find(h, key);
			if (fr.data_i != END_OF_LIST)
				return fr.data_i;

			uint32 i = add_entry(h, key);
			if (fr.data_prev == END_OF_LIST)
				h->_hash[fr.hash_i] = i;
			else
				h->_data[fr.data_prev].next = i;
			return i;
		}

		template<typename T> uint32 make(Hash<T>* h, uint64 key)
		{
			const FindResult fr = find(h, key);
			const uint32 i = add_entry(h, key);

			if (fr.data_prev == END_OF_LIST)
				h->_hash[fr.hash_i] = i;
			else
				h->_data[fr.data_prev].next = i;

			h->_data[i].next = fr.data_i;
			return i;
		}	

		template<typename T> void find_and_erase(Hash<T>* h, uint64 key)
		{
			const FindResult fr = find(h, key);

			if (fr.data_i != END_OF_LIST)
				erase(h, &fr);
		}

		template<typename T> void rehash(Hash<T>* h, uint32 new_size)
		{
			Hash<T> nh;
			hash::init(&nh, h->_hash.allocator);
			vector::resize(&nh._hash, new_size);
			vector::reserve(&nh._data, h->_data.size);
			for (uint32 i=0; i<new_size; ++i)
				nh._hash[i] = END_OF_LIST;
			for (uint32 i=0; i<h->_data.size; ++i) {
				const typename Hash<T>::Entry &e = h->_data[i];
				multi_hash::insert(&nh, e.key, e.value);
			}

			Hash<T> empty;
			hash::init(&empty, h->_data.allocator);
			hash::deinit(h);
			memcpy(h, &nh, sizeof(Hash<T>));
			memcpy(&nh, &empty, sizeof(Hash<T>));
		}

		template<typename T> bool full(const Hash<T>* h)
		{
			const real32 max_load_factor = 0.7f;
			return h->_data.size >= h->_hash.size * max_load_factor;
		}

		template<typename T> void grow(Hash<T>* h)
		{
			const uint32 new_size = h->_data.size * 2 + 10;
			rehash(h, new_size);
		}
	}

	namespace hash
	{
		template<typename T> inline void init(Hash<T>* h, Allocator* allocator)
		{
			memset(h, 0, sizeof(Hash<T>));
			vector::init(&h->_hash, allocator);
			vector::init(&h->_data, allocator);
		}

		template<typename T> inline void copy(Hash<T>* from, Hash<T>* to)
		{
			const uint32 n = from._size;
			vector::copy(&from._hash, &to._hash);
			vector::copy(&from._data, &to._data);
		}

		template<typename T> inline void deinit(Hash<T>* h)
		{
			vector::deinit(&h->_hash);
			vector::deinit(&h->_data);
		}

		template<typename T> bool has(const Hash<T>* h, uint64 key)
		{
			return hash_internal::find_or_fail(h, key) != hash_internal::END_OF_LIST;
		}

		template<typename T> const T& get(const Hash<T>* h, uint64 key, const T& default)
		{
			const uint32 i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? default : h->_data[i].value;
		}

		template<typename T> Option<T> try_get(const Hash<T>* h, uint64 key)
		{
			const uint32 i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? option::none<T>() : option::some(h->_data[i].value);
		}

		template<typename T> const T& get(const Hash<T>* h, uint64 key)
		{
			const uint32 i = hash_internal::find_or_fail(h, key);
			return h->_data[i].value;
		}

		template<typename T> void set(Hash<T>* h, uint64 key, const T &value)
		{
			if (h->_hash.size == 0)
				hash_internal::grow(h);

			const uint32 i = hash_internal::find_or_make(h, key);
			h->_data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T>* h, uint64 key)
		{
			hash_internal::find_and_erase(h, key);
		}

		template<typename T> void reserve(Hash<T>* h, uint32 size)
		{
			hash_internal::rehash(h, size);
		}

		template<typename T> void clear(Hash<T>* h)
		{
			vector::clear(h->_data);
			vector::clear(h->_hash);
		}

		template<typename T> const typename Hash<T>::Entry* begin(const Hash<T>* h)
		{
			return h->_data.data;
		}

		template<typename T> const typename Hash<T>::Entry* end(const Hash<T>* h)
		{
			return vector::last(&h->_data);
		}
	}

	namespace multi_hash
	{
		template<typename T> const typename Hash<T>::Entry *find_first(const Hash<T>* h, uint64 key)
		{
			const uint32 i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? 0 : h->_data[i];
		}

		template<typename T> const typename Hash<T>::Entry *find_next(const Hash<T>* h, const typename Hash<T>::Entry *e)
		{
			uint32 i = e->next;
			while (i != hash_internal::END_OF_LIST) {
				if (h->_data[i].key == e->key)
					return &h->_data[i];
				i = h->_data[i].next;
			}
			return 0;
		}

		template<typename T> uint32 count(const Hash<T>* h, uint64 key)
		{
			uint32 i = 0;
			const typename Hash<T>::Entry *e = find_first(h, key);
			while (e) {
				++i;
				e = find_next(h, e);
			}
			return i;
		}

		template<typename T> void get(const Hash<T>* h, uint64 key, Vector<T> &items)
		{
			const typename Hash<T>::Entry *e = find_first(h, key);
			while (e) {
				vector::push(items, e->value);
				e = find_next(h, e);
			}
		}

		template<typename T> void insert(Hash<T>* h, uint64 key, const T &value)
		{
			if (h->_hash.size == 0)
				hash_internal::grow(h);

			const uint32 i = hash_internal::make(h, key);
			h->_data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T>* h, const typename Hash<T>::Entry *e)
		{
			const hash_internal::FindResult fr = hash_internal::find(h, e);
			if (fr.data_i != hash_internal::END_OF_LIST)
				hash_internal::erase(h, fr);
		}

		template<typename T> void remove_all(Hash<T>* h, uint64 key)
		{
			while (hash::has(h, key))
				hash::remove(h, key);
		}
	}
}