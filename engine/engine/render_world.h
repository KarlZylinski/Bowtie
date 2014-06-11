#pragma once

#include <foundation/collection_types.h>

#include "render_resource_handle.h"

namespace bowtie
{

class RenderWorld
{
public:
	RenderWorld(Allocator& allocator, RenderResourceHandle render_target);

	void add_drawable(RenderResourceHandle drawable);
	void remove_drawable(RenderResourceHandle drawable);
	RenderResourceHandle render_target() const;

	const Array<RenderResourceHandle>& drawables() const;
private:
	Array<RenderResourceHandle> _drawables;
	RenderResourceHandle _render_target;
};

};