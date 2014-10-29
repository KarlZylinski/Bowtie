#pragma once

#include <foundation/vector4.h>
#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include "../../rect.h"
#include "../../render_resource_handle.h"

namespace bowtie
{

class Allocator;
class RenderInterface;

typedef unsigned Entity;

struct RectangleRendererComponent
{
	// Maps entity id to rectangle renderer components
	Hash<unsigned> map;
	unsigned num;
	unsigned capacity;
	// Maybe skip dirtyness and do statereflection right away? But if I can get away with doing deferred reflection
	// checks on only the touched ones, then maybe it's alright.
	unsigned last_dirty_index;
	void* buffer;
	Color* color;
	Rect* rect;
	RenderResourceHandle* render_handle;
};

namespace rectangle_renderer_component
{
	void init(RectangleRendererComponent& c, Allocator& allocator);
	void deinit(RectangleRendererComponent& c, Allocator& allocator);
	void create(RectangleRendererComponent& c, Entity e, Allocator& allocator);
	void destroy(RectangleRendererComponent& c, Entity e);
	void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect);
	const Rect& rect(RectangleRendererComponent& c, Entity e);
}

}