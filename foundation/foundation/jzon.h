#ifndef _JZON_H_
#define _JZON_H_

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JzonValue
{
	bool is_string : 1;
	bool is_int : 1;
	bool is_float : 1;
	bool is_object : 1;
	bool is_array : 1;
	bool is_bool : 1;
	bool is_null : 1;
} JzonValue;

typedef void* (*jzon_allocate)(size_t);
typedef void (*jzon_deallocate)(void*);

typedef struct JzonAllocator {
	jzon_allocate allocate;
	jzon_deallocate deallocate;
} JzonAllocator;

// Parse using default malloc allocator.
JzonValue* jzon_parse(const char* input);

// Parse using custom allocator. Make sure to call jzon_free_custom_allocator using the same allocator.
JzonValue* jzon_parse_custom_allocator(const char* input, JzonAllocator* allocator);

// Find object value by key. Returns NULL if object is not an actual jzon object or there exists no value with the specified key. Uses a binary search algorithm.
//JzonValue* jzon_get(JzonValue* object, const char* key);

unsigned jzon_size(JzonValue* jzon);
char* jzon_key(JzonValue* jzon, unsigned i);
JzonValue* jzon_get(JzonValue* jzon, unsigned i);
JzonValue* jzon_value(JzonValue* jzon, const char* key);
int jzon_int(JzonValue* jzon);
float jzon_float(JzonValue* jzon);
char* jzon_string(JzonValue* jzon);
bool jzon_bool(JzonValue* jzon);

#ifdef __cplusplus
}
#endif

#endif
