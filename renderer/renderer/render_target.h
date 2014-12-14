#pragma once

#include "render_resource.h"
#include "render_texture.h"

namespace bowtie
{

struct RenderTarget
{
    RenderTexture texture;
    RenderResource handle;
};

}