#pragma once

#include <foundation/vector2u.h>

#include "image.h"
#include "resource_handle.h"

namespace bowtie
{

struct RenderTexture
{
	image::PixelFormat pixel_format;
	ResourceHandle render_handle;
	Vector2u resolution;
};

}