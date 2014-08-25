#pragma once

#include "types.h"

namespace bowtie
{
	/// Implementation of the 64 bit MurmurHash2 function
	/// http://murmurhash.googlepages.com/
	uint64_t hash_str(const char* str);
}