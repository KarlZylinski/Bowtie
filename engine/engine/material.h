#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct Shader;
struct Material
{
    RenderResourceHandle render_handle;
    Shader* shader;
};

}