#pragma once

#include "vector.h"
#include "collection_types.h"
#include "option.h"

namespace bowtie {

	/// The hash function stores its data in a "list-in-an-array" where
	/// indices are used instead of pointers. 
	///
	/// When items are removed, the array-list is repacked to always keep
	/// it tightly ordered.

	namespace hash
	{
		template<typename T> void init(Hash<T>& a, Allocator& allocator);
		template<typename T> void copy(Hash<T>& from, Hash<T>& to);
		template<typename T> void deinit(Hash<T>& a);

		/// Returns true if the specified key exists in the hash.
		template<typename T> bool has(const Hash<T> &h, uint64_t key);

		/// Returns the value stored for the specified key, or deffault if the key
		/// does not exist in the hash.
		template<typename T> const T &get(const Hash<T> &h, uint64_t key, const T &default);

		/// Sets the value for the key.
		template<typename T> void set(Hash<T> &h, uint64_t key, const T &value);

		/// Removes the key from the hash if it exists.
		template<typename T> void remove(Hash<T> &h, uint64_t key);

		/// Resizes the hash lookup table to the specified size.
		/// (The table will grow automatically when 70 % full.)
		template<typename T> void reserve(Hash<T> &h, uint32_t size);

		/// Remove all elements from the hash.
		template<typename T> void clear(Hash<T> &h);

		/// Returns a pointer to the first entry in the hash table, can be used to
		/// efficiently iterate over the elements (in random order).
		template<typename T> const typename Hash<T>::Entry *begin(const Hash<T> &h);
		template<typename T> const typename Hash<T>::Entry *end(const Hash<T> &h);
	}

	namespace multi_hash
	{
		/// Finds the first entry with the specified key.
		template<typename T> const typename Hash<T>::Entry *find_first(const Hash<T> &h, uint64_t key);

		/// Finds the next entry with the same key as e.
		template<typename T> const typename Hash<T>::Entry *find_next(const Hash<T> &h, const typename Hash<T>::Entry *e);

		/// Returns the number of entries with the key.
		template<typename T> uint32_t count(const Hash<T> &h, uint64_t key);

		/// Returns all the entries with the specified key.
		/// Use a TempAllocator for the array to avoid allocating memory.
		template<typename T> void get(const Hash<T> &h, uint64_t key, Vector<T> &items);

		/// Inserts the value as an aditional value for the key.
		template<typename T> void insert(Hash<T> &h, uint64_t key, const T &value);

		/// Removes the specified entry.
		template<typename T> void remove(Hash<T> &h, const typename Hash<T>::Entry *e);

		/// Removes all entries with the specified key.
		template<typename T> void remove_all(Hash<T> &h, uint64_t key);
	}

	namespace hash_internal
	{
		const uint32_t END_OF_LIST = 0xffffffffu;
		
		struct FindResult
		{
			uint32_t hash_i;
			uint32_t data_prev;
			uint32_t data_i;
		};	

		template<typename T> uint32_t add_entry(Hash<T> &h, uint64_t key)
		{
			typename Hash<T>::Entry e;
			e.key = key;
			e.next = END_OF_LIST;
			uint32_t ei = h._data.size;
			vector::push(&h._data, e);
			return ei;
		}

		template<typename T> void erase(Hash<T> &h, const FindResult &fr)
		{
			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = h._data[fr.data_i].next;
			else
				h._data[fr.data_prev].next = h._data[fr.data_i].next;

			if (fr.data_i == h._data.size - 1) {
				vector::pop(&h._data);
				return;
			}

			h._data[fr.data_i] = h._data[h._data.size - 1];
			FindResult last = find(h, h._data[fr.data_i].key);

			if (last.data_prev != END_OF_LIST)
				h._data[last.data_prev].next = fr.data_i;
			else
				h._hash[last.hash_i] = fr.data_i;
		}

		template<typename T> FindResult find(const Hash<T> &h, uint64_t key)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (h._hash.size == 0)
				return fr;

			fr.hash_i = key % h._hash.size;
			fr.data_i = h._hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (h._data[fr.data_i].key == key)
					return fr;
				fr.data_prev = fr.data_i;
				fr.data_i = h._data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> FindResult find(const Hash<T> &h, const typename Hash<T>::Entry *e)
		{
			FindResult fr;
			fr.hash_i = END_OF_LIST;
			fr.data_prev = END_OF_LIST;
			fr.data_i = END_OF_LIST;

			if (vector::size(h._hash) == 0)
				return fr;

			fr.hash_i = e->key % vector::size(h._hash);
			fr.data_i = h._hash[fr.hash_i];
			while (fr.data_i != END_OF_LIST) {
				if (&h._data[fr.data_i] == e)
					return fr;
				fr.data_prev = fr.data_i;
				fr.data_i = h._data[fr.data_i].next;
			}
			return fr;
		}

		template<typename T> uint32_t find_or_fail(const Hash<T> &h, uint64_t key)
		{
			return find(h, key).data_i;
		}

		template<typename T> uint32_t find_or_make(Hash<T> &h, uint64_t key)
		{
			const FindResult fr = find(h, key);
			if (fr.data_i != END_OF_LIST)
				return fr.data_i;

			uint32_t i = add_entry(h, key);
			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = i;
			else
				h._data[fr.data_prev].next = i;
			return i;
		}

		template<typename T> uint32_t make(Hash<T> &h, uint64_t key)
		{
			const FindResult fr = find(h, key);
			const uint32_t i = add_entry(h, key);

			if (fr.data_prev == END_OF_LIST)
				h._hash[fr.hash_i] = i;
			else
				h._data[fr.data_prev].next = i;

			h._data[i].next = fr.data_i;
			return i;
		}	

		template<typename T> void find_and_erase(Hash<T> &h, uint64_t key)
		{
			const FindResult fr = find(h, key);
			if (fr.data_i != END_OF_LIST)
				erase(h, fr);
		}

		template<typename T> void rehash(Hash<T> &h, uint32_t new_size)
		{
			Hash<T> nh;
			hash::init(nh, *h._hash.allocator);
			vector::resize(&nh._hash, new_size);
			vector::reserve(&nh._data, h._data.size);
			for (uint32_t i=0; i<new_size; ++i)
				nh._hash[i] = END_OF_LIST;
			for (uint32_t i=0; i<h._data.size; ++i) {
				const typename Hash<T>::Entry &e = h._data[i];
				multi_hash::insert(nh, e.key, e.value);
			}

			Hash<T> empty;
			hash::init(empty, *h._data.allocator);
			hash::deinit(h);
			memcpy(&h, &nh, sizeof(Hash<T>));
			memcpy(&nh, &empty, sizeof(Hash<T>));
		}

		template<typename T> bool full(const Hash<T> &h)
		{
			const float max_load_factor = 0.7f;
			return h._data.size >= h._hash.size * max_load_factor;
		}

		template<typename T> void grow(Hash<T> &h)
		{
			const uint32_t new_size = h._data.size * 2 + 10;
			rehash(h, new_size);
		}
	}

	namespace hash
	{
		template<typename T> inline void init(Hash<T>& a, Allocator& allocator)
		{
			memset(&a, 0, sizeof(Hash<T>));
			vector::init(&a._hash, &allocator);
			vector::init(&a._data, &allocator);
		}

		template<typename T> inline void copy(Hash<T>& from, Hash<T>& to)
		{
			const uint32_t n = from._size;
			vector::copy(&from._hash, &to._hash);
			vector::copy(&from._data, &to._data);
		}

		template<typename T> inline void deinit(Hash<T>& a)
		{
			vector::deinit(&a._hash);
			vector::deinit(&a._data);
		}

		template<typename T> bool has(const Hash<T> &h, uint64_t key)
		{
			return hash_internal::find_or_fail(h, key) != hash_internal::END_OF_LIST;
		}

		template<typename T> const T &get(const Hash<T> &h, uint64_t key, const T &default)
		{
			const uint32_t i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? default : h._data[i].value;
		}

		template<typename T> Option<T> try_get(const Hash<T> h, uint64_t key)
		{
			const uint32_t i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? option::none<T>() : option::some(h._data[i].value);
		}

		template<typename T> const T &get(const Hash<T> &h, uint64_t key)
		{
			const uint32_t i = hash_internal::find_or_fail(h, key);
			return h._data[i].value;
		}

		template<typename T> void set(Hash<T> &h, uint64_t key, const T &value)
		{
			if (h._hash.size == 0)
				hash_internal::grow(h);

			const uint32_t i = hash_internal::find_or_make(h, key);
			h._data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T> &h, uint64_t key)
		{
			hash_internal::find_and_erase(h, key);
		}

		template<typename T> void reserve(Hash<T> &h, uint32_t size)
		{
			hash_internal::rehash(h, size);
		}

		template<typename T> void clear(Hash<T> &h)
		{
			vector::clear(h._data);
			vector::clear(h._hash);
		}

		template<typename T> const typename Hash<T>::Entry *begin(const Hash<T> &h)
		{
			return h._data.data;
		}

		template<typename T> const typename Hash<T>::Entry *end(const Hash<T> &h)
		{
			return vector::last(&h._data);
		}
	}

	namespace multi_hash
	{
		template<typename T> const typename Hash<T>::Entry *find_first(const Hash<T> &h, uint64_t key)
		{
			const uint32_t i = hash_internal::find_or_fail(h, key);
			return i == hash_internal::END_OF_LIST ? 0 : &h._data[i];
		}

		template<typename T> const typename Hash<T>::Entry *find_next(const Hash<T> &h, const typename Hash<T>::Entry *e)
		{
			uint32_t i = e->next;
			while (i != hash_internal::END_OF_LIST) {
				if (h._data[i].key == e->key)
					return &h._data[i];
				i = h._data[i].next;
			}
			return 0;
		}

		template<typename T> uint32_t count(const Hash<T> &h, uint64_t key)
		{
			uint32_t i = 0;
			const typename Hash<T>::Entry *e = find_first(h, key);
			while (e) {
				++i;
				e = find_next(h, e);
			}
			return i;
		}

		template<typename T> void get(const Hash<T> &h, uint64_t key, Vector<T> &items)
		{
			const typename Hash<T>::Entry *e = find_first(h, key);
			while (e) {
				vector::push(items, e->value);
				e = find_next(h, e);
			}
		}

		template<typename T> void insert(Hash<T> &h, uint64_t key, const T &value)
		{
			if (h._hash.size == 0)
				hash_internal::grow(h);

			const uint32_t i = hash_internal::make(h, key);
			h._data[i].value = value;
			if (hash_internal::full(h))
				hash_internal::grow(h);
		}

		template<typename T> void remove(Hash<T> &h, const typename Hash<T>::Entry *e)
		{
			const hash_internal::FindResult fr = hash_internal::find(h, e);
			if (fr.data_i != hash_internal::END_OF_LIST)
				hash_internal::erase(h, fr);
		}

		template<typename T> void remove_all(Hash<T> &h, uint64_t key)
		{
			while (hash::has(h, key))
				hash::remove(h, key);
		}
	}
}