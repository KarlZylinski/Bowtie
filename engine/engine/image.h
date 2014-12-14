#pragma once

#include "render_resource_handle.h"
#include <base/vector2u.h>

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
    uint32 data_size;
    PixelFormat pixel_format;
    Vector2u resolution;
};

}