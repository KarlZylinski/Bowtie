#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct RenderTarget
{
	RenderTarget(RenderResourceHandle render_texture, RenderResourceHandle target_handle) : render_texture(render_texture), target_handle(target_handle) {}
	RenderResourceHandle render_texture;
	RenderResourceHandle target_handle;
};


}