#pragma once

#include "collection_types.h"

namespace bowtie
{
	namespace vector
	{
		void* init(VectorHeader* header, Allocator* allocator, unsigned element_size, unsigned element_alignment);
		void clear(VectorHeader* header);
		void* push(VectorHeader* header, void* vector, const void* item);
		//void remove(VectorHeader* header, void** vector)
	}
}