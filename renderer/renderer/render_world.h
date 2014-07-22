#pragma once

#include <foundation/collection_types.h>

#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
class RenderWorld
{
public:
	RenderWorld(Allocator& allocator, RenderResourceHandle render_target);
	~RenderWorld();

	void add_drawable(RenderResourceHandle drawable);
	void remove_drawable(RenderResourceHandle drawable);
	RenderResourceHandle render_target() const;

	const Array<RenderResourceHandle>& drawables() const;
private:
	RenderWorld& operator=(const RenderWorld&);
	RenderWorld(const RenderWorld&);

	Allocator& _allocator;
	Array<RenderResourceHandle> _drawables;
	RenderResourceHandle _render_target;
};

};