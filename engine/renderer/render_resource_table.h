#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct RenderResource;

namespace render_resource_table
{
    void free(RenderResource* table, RenderResourceHandle handle);
    RenderResource lookup(const RenderResource* table, RenderResourceHandle handle);
    void set(RenderResource* table, RenderResourceHandle handle, const RenderResource* resource);
}

} // namespace bowtie;