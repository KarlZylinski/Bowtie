#pragma once

namespace bowtie
{
	struct Matrix4
	{
		float values[4][4];

		float* operator[](unsigned i){
			return values[i];
		}
	};
} // namespace bowtie