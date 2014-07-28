#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
struct RenderTexture;
struct RenderTarget
{
	RenderTarget(Allocator& allocator, RenderTexture* render_texture, RenderResourceHandle target_handle);
	~RenderTarget();
	Allocator& allocator;
	RenderTexture* render_texture;
	RenderResourceHandle target_handle;
	RenderTarget& operator=(const RenderTarget&);
};


}