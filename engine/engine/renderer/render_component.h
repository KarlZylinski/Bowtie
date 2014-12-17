#pragma once

#include <base/vector4.h>
#include <base/matrix4.h>
#include <base/quad.h>

namespace bowtie
{

struct RenderComponent
{
    RenderResourceHandle material;
    Color color;
    Quad geometry;
    int32 depth;
};

}