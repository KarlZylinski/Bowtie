#include "rectangle_geometry.h"
#include "texture.h"
#include <foundation/memory.h>
#include <string.h>

namespace bowtie
{

namespace
{
void update_geometry(const Color& color, void* data, const Rect& rect);
}

////////////////////////////////
// Public interface.

RectangleGeometry::RectangleGeometry(const Color& color, const Rect& rect): _color(color), _rect(rect)
{
	update_geometry(_color, _data, _rect);
}

const IDrawableGeometry& RectangleGeometry::clone(Allocator& allocator) const
{
	return *allocator.construct<RectangleGeometry>(*this);
}

IDrawableGeometry& RectangleGeometry::clone(Allocator& allocator)
{
	return *allocator.construct<RectangleGeometry>(*this);
}

const Vector4& RectangleGeometry::color() const
{
	return _color;
}

const float* RectangleGeometry::data() const
{
	return _data;
}

unsigned RectangleGeometry::data_size() const
{
	return s_geometry_size;
}

bool RectangleGeometry::has_changed() const
{
	return _has_changed;
}

void RectangleGeometry::reset_has_changed()
{
	_has_changed = false;
}
	
const Rect& RectangleGeometry::rect() const
{
	return _rect;
}

void RectangleGeometry::set_color(const Vector4& color)
{
	if (_color == color)
		return;

	_color = color;
	update_geometry(_color, _data, _rect);
	_has_changed = true;
}

void RectangleGeometry::set_rect(const Rect& rect)
{
	_rect = rect;
	update_geometry(_color, _data, _rect);
	_has_changed = true;
}

const Texture* RectangleGeometry::texture() const
{
	return nullptr;
}


////////////////////////////////
// Implementation.

namespace
{

void update_geometry(const Color& color, void* data, const Rect& rect)
{	
	auto w = (float)rect.size.x;
	auto h = (float)rect.size.y;
	
	float geometry[54] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f,
		color.x, color.y, color.z, color.w,
		w, 0.0f, 0.0f,
		1.0f, 0.0f,
		color.x, color.y, color.z, color.w,
		0.0f, h, 0.0f,
		0.0f, 1.0f,
		color.x, color.y, color.z, color.w,

		w, 0.0f, 0.0f,
		1.0f, 0.0f,
		color.x, color.y, color.z, color.w,
		w, h, 0.0f,
		1.0f, 1.0f,
		color.x, color.y, color.z, color.w,
		0.0f, h, 0.0f,
		0.0f, 1.0f,
		color.x, color.y, color.z, color.w
	};

	memcpy((void*)data, geometry, RectangleGeometry::s_geometry_size);
}

} // implementation

} // namespace bowtie
