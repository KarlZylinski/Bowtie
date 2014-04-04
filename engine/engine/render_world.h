#pragma once

#include <foundation/collection_types.h>

#include "render_resource_handle.h"

namespace bowtie
{

class RenderWorld
{
public:
	RenderWorld(Allocator& allocator);

	void add_sprite(RenderResourceHandle sprite);
	void remove_sprite(RenderResourceHandle sprite);

	const Array<RenderResourceHandle>& sprites() const;
private:
	Array<RenderResourceHandle> _sprites;
};

};