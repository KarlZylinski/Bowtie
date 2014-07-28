#include "render_target.h"
#include <foundation/memory.h>

namespace bowtie
{

RenderTarget::RenderTarget(Allocator& allocator, RenderTexture* render_texture, RenderResourceHandle target_handle) : allocator(allocator), render_texture(render_texture), target_handle(target_handle)
{
}

RenderTarget::~RenderTarget()
{
	allocator.deallocate(render_texture);
}

}
