#pragma once

#include "render_resource_handle.h"
#include <foundation/vector2u.h>

namespace bowtie
{

enum class PixelFormat
{
	RGB,
	RGBA
};

struct Image
{
	void* data;
	unsigned data_size;
	PixelFormat pixel_format;
	Vector2u resolution;
};

}