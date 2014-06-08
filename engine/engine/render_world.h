#pragma once

#include <foundation/collection_types.h>

#include "render_resource_handle.h"

namespace bowtie
{

class RenderWorld
{
public:
	RenderWorld(Allocator& allocator);

	void add_drawable(RenderResourceHandle drawable);
	void remove_drawable(RenderResourceHandle drawable);

	const Array<RenderResourceHandle>& drawables() const;
private:
	Array<RenderResourceHandle> _drawables;
};

};