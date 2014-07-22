#include "render_world.h"

#include <foundation/array.h>

#include "render_target.h"

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator, RenderResourceHandle render_target) : _allocator(allocator), _drawables(allocator), _render_target(render_target)
{
}

RenderWorld::~RenderWorld()
{
	MAKE_DELETE(_allocator, RenderTarget, (RenderTarget*)_render_target.render_object);
}

void RenderWorld::add_drawable(RenderResourceHandle drawable)
{
	array::push_back(_drawables, drawable);
}

void RenderWorld::remove_drawable(RenderResourceHandle )
{

}

RenderResourceHandle RenderWorld::render_target() const
{
	return _render_target;
}

const Array<RenderResourceHandle>& RenderWorld::drawables() const
{
	return _drawables;
}

} // namespace bowtie