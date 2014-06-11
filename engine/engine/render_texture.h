#pragma once

#include <foundation/vector2u.h>

#include "image.h"
#include "render_resource_handle.h"

namespace bowtie
{

struct RenderTexture
{
	image::PixelFormat pixel_format;
	RenderResourceHandle render_handle;
	Vector2u resolution;
};

}