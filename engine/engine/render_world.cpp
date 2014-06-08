#include "render_world.h"

#include <foundation/array.h>

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator) : _drawables(allocator)
{
}

void RenderWorld::add_drawable(RenderResourceHandle drawable)
{
	array::push_back(_drawables, drawable);
}

void RenderWorld::remove_drawable(RenderResourceHandle )
{

}

const Array<RenderResourceHandle>& RenderWorld::drawables() const
{
	return _drawables;
}

} // namespace bowtie