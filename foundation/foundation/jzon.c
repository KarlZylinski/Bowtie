#include "jzon.h"
#include <stdlib.h>
#include <string.h>


// Hash function used for hashing object keys.
// From http://murmurhash.googlepages.com/

uint64_t hash_str(const char* str, size_t len)
{
	uint64_t seed = 0;

	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	const uint32_t r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)str;
	const uint64_t * end = data + (len / 8);

	while (data != end)
	{
#ifdef PLATFORM_BIG_ENDIAN
		uint64_t k = *data++;
		char *p = (char *)&k;
		char c;
		c = p[0]; p[0] = p[7]; p[7] = c;
		c = p[1]; p[1] = p[6]; p[6] = c;
		c = p[2]; p[2] = p[5]; p[5] = c;
		c = p[3]; p[3] = p[4]; p[4] = c;
#else
		uint64_t k = *data++;
#endif

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch (len & 7)
	{
	case 7: h ^= ((uint64_t)data2[6]) << 48;
	case 6: h ^= ((uint64_t)data2[5]) << 40;
	case 5: h ^= ((uint64_t)data2[4]) << 32;
	case 4: h ^= ((uint64_t)data2[3]) << 24;
	case 3: h ^= ((uint64_t)data2[2]) << 16;
	case 2: h ^= ((uint64_t)data2[1]) << 8;
	case 1: h ^= ((uint64_t)data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}


// Jzon implmenetation

typedef struct OutputBuffer {
	unsigned size;
	char* data;
	char* write_head;
} OutputBuffer;

unsigned current_write_offset(OutputBuffer* output)
{
	return (unsigned)(output->write_head - output->data);
}

void grow(OutputBuffer* output, unsigned additional_size, JzonAllocator* allocator)
{
	unsigned write_offset = current_write_offset(output);
	unsigned new_size = output->size * 2 + additional_size;
	char* new_data = (char*)allocator->allocate(new_size);
	memcpy(new_data, output->data, output->size);
	allocator->deallocate(output->data);
	output->data = new_data;
	output->write_head = new_data + write_offset;
	output->size = new_size;
}

void write(OutputBuffer* output, void* data, unsigned size, JzonAllocator* allocator)
{
	if (current_write_offset(output) + size > output->size)
		grow(output, size, allocator);

	memcpy(output->write_head, data, size);
	output->write_head += size;
}

void advance(OutputBuffer* output, unsigned len, JzonAllocator* allocator)
{
	if (current_write_offset(output) + len > output->size)
		grow(output, len, allocator);

	output->write_head += len;
}

unsigned offset_from_offset(OutputBuffer* output, unsigned offset)
{
	return current_write_offset(output) - offset;
}

void next(const char** input)
{
	++*input;
}

char current(const char** input)
{
	return **input;
}

bool is_multiline_string_quotes(const char* str)
{
	return *str != '\0' && *str == '"' && *(str + 1) != '\0' && *(str + 1) == '"' && *(str + 2) != '\0' &&  *(str + 2) == '"';
}

void skip_whitespace(const char** input)
{
	while (current(input))
	{
		while (current(input) && (current(input) <= ' ' || current(input) == ','))
			next(input);
		
		// Skip comment.
		if (current(input) == '#')
		{
			while (current(input) && current(input) != '\n')
				next(input);
		}
		else
			break;
	}
};

uint64_t parse_multiline_string(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	if (!is_multiline_string_quotes(*input))
		return (uint64_t)-1;
	
	*input += 3;
	unsigned output_start_offset = current_write_offset(output);
	char* end = (char*)*input;
	char* row_start = (char*)*input;

	while (current(input))
	{
		if (current(input) == '\n' || current(input) == '\r') {
			unsigned str_len = (unsigned)(end - row_start);

			if (str_len != 0)
			{
				write(output, row_start, str_len, allocator);
				char line_break = '\n';
				write(output, &line_break, sizeof(char), allocator);
			}

			skip_whitespace(input);
			row_start = (char*)*input;
			end = (char*)*input;
		}

		if (is_multiline_string_quotes(*input))
		{
			*input += 3;
			break;
		}

		++end;
		next(input);
	}

	unsigned str_len = current_write_offset(output) - output_start_offset;
	char termination = '\0';
	write(output, &termination, sizeof(char), allocator);
	return hash_str((char*)(output->data + output_start_offset), str_len);
}

uint64_t parse_string_internal(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	if (current(input) != '"')
		return (uint64_t)-1;

	if (is_multiline_string_quotes(*input))
		return parse_multiline_string(input, output, allocator);

	next(input);

	char* start = (char*)*input;
	char* end = start;

	while (current(input))
	{
		if (current(input) == '"')
		{
			next(input);
			break;
		}
		if (current(input) == '\\')
		{
			next(input);
			if (current(input) == 'u')
			{
				char* before_this_char = ((char*)*input) - 2;
				next(input);
				long charcode_l = strtol(*input, NULL, 16);

				if (charcode_l > 255)
					charcode_l = 0;

				char charcode = (char)charcode_l;

				if (before_this_char > start)
				{
					unsigned str_len = (unsigned)(before_this_char - start);
					write(output, start, str_len, allocator);
				}

				write(output, (char*)&charcode, sizeof(char), allocator);
				start = (char*)*input;
				end = start;
			}
			else
			{
				char* before_this_char = (char*)*input - 1;

				if (before_this_char > start)
				{
					unsigned str_len = (unsigned)(before_this_char - start);
					write(output, start, str_len, allocator);
				}

				char c = current(input);
				write(output, &c, sizeof(char), allocator);
				start = (char*)*input;
				end = start;
			}
		}

		++end;
		next(input);
	}

	unsigned str_len = (unsigned)(end - start);
	write(output, start, str_len, allocator);
	char termination = '\0';
	write(output, &termination, sizeof(char), allocator);
	return hash_str(start, str_len);
}

uint64_t parse_keyname(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	if (current(input) == '"')
		return parse_string_internal(input, output, allocator);
		
	char* start = (char*)*input;
	char* end = start;

	while (current(input))
	{
		if (current(input) == ':')
			break;
		else
			++end;

		next(input);
	}

	unsigned key_len = (unsigned)(end - start);
	write(output, start, key_len, allocator);
	char termination = '\0';
	write(output, &termination, sizeof(char), allocator);
	return hash_str(start, key_len);
}

int parse_value(const char** input, OutputBuffer* output, JzonAllocator* allocator);

int parse_string(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	{
		JzonValue* header = (JzonValue*)output->write_head;
		memset(header, 0, sizeof(JzonValue));
		header->is_string = true;
	}

	advance(output, sizeof(JzonValue), allocator);
	parse_string_internal(input, output, allocator);
	return 0;
}

int parse_array(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{	
	if (current(input) != '[')
		return -1;
	
	{
		JzonValue* header = (JzonValue*)output->write_head;
		memset(header, 0, sizeof(JzonValue));
		header->is_array = true;
	}

	unsigned header_offset = current_write_offset(output);
	advance(output, sizeof(JzonValue), allocator);
	unsigned offset_table_offset_write_pos = current_write_offset(output);
	advance(output, sizeof(unsigned), allocator);
	next(input);

	// Empty array.
	if (current(input) == ']')
	{
		*(unsigned*)(output->data + offset_table_offset_write_pos) = offset_from_offset(output, header_offset);
		unsigned size = 0;
		write(output, &size, sizeof(size), allocator);
		return 0;
	}

	OutputBuffer offset_table;
	offset_table.data = (char*)allocator->allocate(4);
	offset_table.size = 4;
	offset_table.write_head = offset_table.data + sizeof(unsigned);
	*(unsigned*)offset_table.data = 0;
	
	while (current(input))
	{
		skip_whitespace(input);
		unsigned value_offset = offset_from_offset(output, header_offset);
		int error = parse_value(input, output, allocator);

		if (error != 0)
			return error;

		write(&offset_table, &value_offset, sizeof(unsigned), allocator);
		++(*(unsigned*)offset_table.data);
		skip_whitespace(input);

		if (current(input) == ']')
		{
			next(input);
			break;
		}
	}

	*(unsigned*)(output->data + offset_table_offset_write_pos) = offset_from_offset(output, header_offset);
	write(output, offset_table.data, (unsigned)current_write_offset(&offset_table), allocator);
	allocator->deallocate(offset_table.data);
	return 0;
}

typedef struct ObjectOffsetTableEntry {
	uint64_t key_hash;
	unsigned key_offset;
	unsigned value_offset;
} ObjectOffsetTableEntry;

int find_object_pair_insertion_index(ObjectOffsetTableEntry* objects, unsigned size, uint64_t key_hash)
{
	if (size == 0)
		return 0;

	for (unsigned i = 0; i < size; ++i)
	{
		if (objects[i].key_hash > key_hash)
			return i;
	}

	return size;
}

void move_forward_entries_at(OutputBuffer* output, unsigned size, unsigned insertion_point, JzonAllocator* allocator)
{
	if (output->write_head - output->data + sizeof(ObjectOffsetTableEntry) > output->size)
		grow(output, sizeof(ObjectOffsetTableEntry), allocator);

	ObjectOffsetTableEntry* objects = (ObjectOffsetTableEntry*)(output->data + sizeof(unsigned));
	memmove(objects + insertion_point + 1, objects + insertion_point, (size - insertion_point) * sizeof(ObjectOffsetTableEntry));
}

int parse_object(const char** input, OutputBuffer* output, bool root_object, JzonAllocator* allocator)
{
	if (current(input) == '{')
		next(input);
	else if (!root_object)
		return -1;

	{
		JzonValue* header = (JzonValue*)output->write_head;
		memset(header, 0, sizeof(JzonValue));
		header->is_object = true;
	}

	unsigned header_offset = current_write_offset(output);
	advance(output, sizeof(JzonValue), allocator);
	unsigned offset_table_offset_write_pos = current_write_offset(output);
	advance(output, sizeof(unsigned), allocator);
	skip_whitespace(input);
	
	// Empty object.
	if (current(input) == '}')
	{
		*(unsigned*)(output->data + offset_table_offset_write_pos) = offset_from_offset(output, header_offset);
		unsigned size = 0;
		write(output, &size, sizeof(size), allocator);
		return 0;
	}

	OutputBuffer offset_table;
	offset_table.data = (char*)allocator->allocate(4);
	offset_table.size = 4;
	offset_table.write_head = offset_table.data + sizeof(unsigned);
	*(unsigned*)offset_table.data = 0;

	while (current(input))
	{
		skip_whitespace(input);
		unsigned key_offset = offset_from_offset(output, header_offset);
		uint64_t key_hash = parse_keyname(input, output, allocator);
		skip_whitespace(input);
		
		if (key_hash == -1 || current(input) != ':')
			return -1;

		next(input);
		unsigned value_offset = offset_from_offset(output, header_offset);
		int error = parse_value(input, output, allocator);

		if (error != 0)
			return error;
			
		unsigned table_size = *(unsigned*)offset_table.data;
		unsigned insertion_point = find_object_pair_insertion_index((ObjectOffsetTableEntry*)(offset_table.data + sizeof(unsigned)), table_size, key_hash);
		move_forward_entries_at(&offset_table, table_size, insertion_point, allocator);
		ObjectOffsetTableEntry* entry = ((ObjectOffsetTableEntry*)(offset_table.data + sizeof(unsigned)) + insertion_point);
		entry->key_hash = key_hash;
		entry->key_offset = key_offset;
		entry->value_offset = value_offset;
		advance(&offset_table, sizeof(ObjectOffsetTableEntry), allocator);
		++(*(unsigned*)offset_table.data);

		skip_whitespace(input);

		if (current(input) == '}')
		{
			next(input);
			break;
		}
	}
	
	*(unsigned*)(output->data + offset_table_offset_write_pos) = offset_from_offset(output, header_offset);
	write(output, offset_table.data, (unsigned)(offset_table.write_head - offset_table.data), allocator);
	allocator->deallocate(offset_table.data);
	return 0;
}

int parse_number(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	{
		JzonValue* header = (JzonValue*)output->write_head;
		memset(header, 0, sizeof(JzonValue));
	}
	
	unsigned header_offset = current_write_offset(output);
	bool is_float = false;

	advance(output, sizeof(JzonValue), allocator);
	char* start = (char*)*input;
	char* end = start;
	
	if (current(input) == '-')
	{
		++end;
		next(input);
	}

	while (current(input) >= '0' && current(input) <= '9')
	{
		++end;
		next(input);
	}

	if (current(input) == '.')
	{
		is_float = true;
		++end;
		next(input);

		while (current(input) >= '0' && current(input) <= '9')
		{
			++end;
			next(input);
		}
	}

	if (current(input) == 'e' || current(input) == 'E')
	{
		is_float = true;
		++end;
		next(input);

		if (current(input) == '-' || current(input) == '+')
		{
			++end;
			next(input);
		}

		while (current(input) >= '0' && current(input) <= '9')
		{
			++end;
			next(input);
		}
	}

	if (is_float)
	{
		JzonValue* header = (JzonValue*)(output->data + header_offset);
		header->is_float = true;
		float value = (float)strtod(start, &end);
		write(output, &value, sizeof(float), allocator);
	}
	else
	{
		JzonValue* header = (JzonValue*)(output->data + header_offset);
		header->is_int = true;
		int value = (int)strtol(start, &end, 10);
		write(output, &value, sizeof(int), allocator);
	}

	return 0;
}

int parse_word(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	char* start = (char*)*input;
	char* end = start;

	while (current(input))
	{
		if ((current(input) >= 'A' && current(input) <= 'Z') || (current(input) >= 'a' && current(input) <= 'z'))
			++end;
		else
		{
			if ((unsigned)(end - start) == 4 && memcmp(start, "true", 4) == 0)
			{
				JzonValue* header = (JzonValue*)output->write_head;
				memset(header, 0, sizeof(JzonValue));
				header->is_bool = true;
				advance(output, sizeof(JzonValue), allocator);
				bool value = true;
				write(output, &value, sizeof(bool), allocator);
				return 0;
			}
			else if ((unsigned)(end - start) == 5 && memcmp(start, "false", 5) == 0)
			{
				JzonValue* header = (JzonValue*)output->write_head;
				memset(header, 0, sizeof(JzonValue));
				header->is_bool = true;
				advance(output, sizeof(JzonValue), allocator);
				bool value = false;
				write(output, &value, sizeof(bool), allocator);
				return 0;
			}
			else if ((unsigned)(end - start) == 4 && memcmp(start, "null", 4) == 0)
			{
				JzonValue* header = (JzonValue*)output->write_head;
				memset(header, 0, sizeof(JzonValue));
				header->is_null = true;
				advance(output, sizeof(JzonValue), allocator);
				return 0;
			}

			break;
		}

		next(input);
	}

	return -1;
}

int parse_value(const char** input, OutputBuffer* output, JzonAllocator* allocator)
{
	skip_whitespace(input);
	char ch = current(input);

	switch (ch)
	{
		case '{': return parse_object(input, output, false, allocator);
		case '[': return parse_array(input, output, allocator);
		case '"': return parse_string(input, output, allocator);
		case '-': return parse_number(input, output, allocator);
		default: return ch >= '0' && ch <= '9' ? parse_number(input, output, allocator) : parse_word(input, output, allocator);
	}
}


// Public interface

JzonValue* jzon_parse_custom_allocator(const char* input, JzonAllocator* allocator)
{
	unsigned size = (unsigned)strlen(input);
	char* data = (char*)allocator->allocate(size);
	OutputBuffer output;
	output.data = data;
	output.write_head = data;
	output.size = size;
	int error = parse_object(&input, &output, true, allocator);

	if (error != 0)
	{
		allocator->deallocate(output.data);
		return NULL;
	}

	return (JzonValue*)output.data;
}

JzonValue* jzon_parse(const char* input)
{
	JzonAllocator allocator = { malloc, free };
	return jzon_parse_custom_allocator(input, &allocator);
}

char* lookup_table_start(JzonValue* jzon)
{
	return (((char*)jzon) + *(unsigned*)(jzon + 1));
}

unsigned jzon_size(JzonValue* jzon)
{
	if (jzon->is_object || jzon->is_array)
		return *(unsigned*)lookup_table_start(jzon);

	return 0;
}

char* jzon_key(JzonValue* jzon, unsigned i)
{
	return (char*)(((char*)jzon) + ((ObjectOffsetTableEntry*)(lookup_table_start(jzon) + sizeof(unsigned)))[i].key_offset);
}

JzonValue* jzon_get(JzonValue* jzon, unsigned i)
{
	if (jzon->is_object)
		return (JzonValue*)(((char*)jzon) + ((ObjectOffsetTableEntry*)(lookup_table_start(jzon) + sizeof(unsigned)))[i].value_offset);

	if (jzon->is_array)
		return (JzonValue*)(((char*)jzon) + ((unsigned*)(lookup_table_start(jzon) + sizeof(unsigned)))[i]);

	return NULL;
}

int jzon_int(JzonValue* jzon)
{
	return *(int*)(jzon + 1);
}

float jzon_float(JzonValue* jzon)
{
	return *(float*)(jzon + 1);
}

bool jzon_bool(JzonValue* jzon)
{
	return *(bool*)(jzon + 1);
}

char* jzon_string(JzonValue* jzon)
{
	return (char*)(jzon + 1);
}

JzonValue* jzon_value(JzonValue* jzon, const char* key)
{
	if (!jzon->is_object)
		return NULL;

	char* table_start = lookup_table_start(jzon);
	unsigned size = *(unsigned*)table_start;

	if (size == 0)
		return NULL;
	
	uint64_t key_hash = hash_str(key, strlen(key));
	ObjectOffsetTableEntry* offset_table = (ObjectOffsetTableEntry*)(table_start + sizeof(unsigned));

	unsigned first = 0;
	unsigned last = size - 1;
	unsigned middle = (first + last) / 2;

	while (first <= last)
	{
		if (offset_table[middle].key_hash < key_hash)
			first = middle + 1;
		else if (offset_table[middle].key_hash == key_hash)
			return (JzonValue*)((char*)jzon + offset_table[middle].value_offset);
		else
			last = middle - 1;

		middle = (first + last) / 2;
	}

	return NULL;
}
