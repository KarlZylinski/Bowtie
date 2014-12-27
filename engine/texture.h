#pragma once

#include "image.h"
#include "renderer/render_resource_handle.h"

namespace bowtie
{

struct Texture
{
    Image* image;
    RenderResourceHandle render_handle;
};

}