#pragma once

#include "resource_handle.h"
#include <foundation/vector2u.h>

namespace bowtie
{

namespace image
{

enum PixelFormat
{
	RGB
};

}

struct Image
{
	image::PixelFormat pixel_format;
	unsigned data_size;
	void* data;
	Vector2u resolution;
};

}