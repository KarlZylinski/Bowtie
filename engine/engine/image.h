#pragma once

#include "resource_handle.h"
#include <foundation/vector2u.h>

namespace bowtie
{

namespace image
{

enum PixelFormat
{
	RGB,
	RGBA
};

}

struct Image
{
	void* data;
	unsigned data_size;
	image::PixelFormat pixel_format;
	Vector2u resolution;
};

}