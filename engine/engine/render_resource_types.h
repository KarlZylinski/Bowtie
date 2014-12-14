#pragma once

#include <foundation/matrix4.h>

#include "image.h"
#include "uniform.h"
#include "render_resource_handle.h"

namespace bowtie
{

struct RenderResourceData
{
    enum Type
    {
        RenderTarget, Shader, Texture, World, RenderMaterial, SpriteRenderer
    };

    Type type;
    void* data;
};

struct MaterialResourceData
{
    RenderResourceHandle handle;
    RenderResourceHandle shader;
    uint32 num_uniforms;
};
    
struct ShaderResourceData
{
    RenderResourceHandle handle;
    uint32 vertex_shader_source_offset;
    uint32 fragment_shader_source_offset;
};

struct TextureResourceData
{
    RenderResourceHandle handle;
    PixelFormat pixel_format;
    uint32 texture_data_size;
    uint32 texture_data_dynamic_data_offset;
    Vector2u resolution;
};

struct UniformResourceData
{
    uniform::AutomaticValue automatic_value;
    uniform::Type type;
    uint32 name_offset;
    uint32 value_offset;
};

struct CreateSpriteRendererData
{
    RenderResourceHandle world;
    uint32 num;
};

struct UpdateSpriteRendererData
{
    uint32 num;
};

struct RenderWorldResourceData
{
    RenderResourceHandle handle;
};

namespace render_resource_data
{

inline RenderResourceData create(RenderResourceData::Type type)
{
    RenderResourceData rr = { type, 0 };
    return rr;
}

} // render_resource_data

} // namespace bowtie
