#pragma once

#include <cassert>

namespace bowtie
{
	struct Matrix4
	{
		float values[4][4];

		float* operator[](unsigned i){
			return values[i];
		}
	};

	inline Matrix4 operator*(const Matrix4&, const Matrix4&)
	{
		assert(!"Not implemented");
		return Matrix4();
	}

} // namespace bowtie