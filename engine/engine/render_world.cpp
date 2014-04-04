#include "render_world.h"

#include <foundation/array.h>

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator) : _sprites(allocator)
{
}

void RenderWorld::add_sprite(RenderResourceHandle sprite)
{
	array::push_back(_sprites, sprite);
}

void RenderWorld::remove_sprite(RenderResourceHandle )
{

}

const Array<RenderResourceHandle>& RenderWorld::sprites() const
{
	return _sprites;
}

} // namespace bowtie