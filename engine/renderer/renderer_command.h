#pragma once
#include "../rect.h"
#include "render_resource_handle.h"
#include "uniform.h"
#include <base/vector2u.h>
#include <base/vector4.h>
#include <base/matrix4.h>

namespace bowtie
{

struct RendererCommand
{
    enum Type { Fence, RenderWorld, LoadResource, UpdateResource, Resize, CombineRenderedWorlds, SetUniformValue };
    Type type;
    void* data;
    uint32 dynamic_data_size;
    void* dynamic_data;
};

struct RenderWorldData
{
    Rect view;
    RenderResourceHandle render_world;
    real32 time;
};

struct ResizeData
{
    Vector2u resolution;
};

struct SetUniformValueData
{
    RenderResourceHandle material;
    uniform::Type type;
    uint64 uniform_name;
};

}