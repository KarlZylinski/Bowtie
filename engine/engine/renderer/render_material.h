#pragma once
#include "render_resource_handle.h"
#include "render_uniform.h"

namespace bowtie
{

struct RenderMaterial
{
    RenderResourceHandle shader;
    uint32 num_uniforms;
    RenderUniform uniforms[16];
};

namespace render_material
{
    void init(RenderMaterial* material, uint32 num_uniforms, RenderResourceHandle shader);
    void set_uniform_vector4_value(RenderMaterial* material, uint64 name, const Vector4* value);
    void set_uniform_uint32_value(RenderMaterial* material, uint64 name, uint32 value);
    void set_uniform_real32_value(RenderMaterial* material, uint64 name, real32 value);
}

}