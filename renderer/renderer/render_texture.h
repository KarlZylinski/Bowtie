#pragma once

#include <engine/image.h>
#include <foundation/vector2u.h>

#include "render_resource.h"

namespace bowtie
{

struct RenderTexture
{
	image::PixelFormat pixel_format;
	RenderResource render_handle;
	Vector2u resolution;
};

}