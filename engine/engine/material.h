#pragma once

#include "renderer/render_resource_handle.h"

namespace bowtie
{

struct Shader;
struct Material
{
    RenderResourceHandle render_handle;
    Shader* shader;
};

}