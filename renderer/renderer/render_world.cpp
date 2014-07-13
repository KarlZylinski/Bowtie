#include "render_world.h"

#include <foundation/array.h>

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator, RenderResourceHandle render_target) : _drawables(allocator), _render_target(render_target)
{
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