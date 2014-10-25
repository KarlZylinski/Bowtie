#include "sprite_geometry.h"
#include "texture.h"
#include <foundation/memory.h>
#include <string.h>

namespace bowtie
{

namespace
{
void update_geometry(const Color& color, void* data, const Rect& rect, const Texture& texture);
}

////////////////////////////////
// Public interface.

SpriteGeometry::SpriteGeometry(const Texture& texture) : _color(1, 1, 1, 1), _texture(&texture)
{
	_rect = Rect(Vector2(), Vector2((float)_texture->image->resolution.x, (float)_texture->image->resolution.y));
	update_geometry(_color, _data, _rect, *_texture);
}

const IDrawableGeometry& SpriteGeometry::clone(Allocator& allocator) const
{
	return *allocator.construct<SpriteGeometry>(*this);
}

IDrawableGeometry& SpriteGeometry::clone(Allocator& allocator)
{
	return *allocator.construct<SpriteGeometry>(*this);
}

const Vector4& SpriteGeometry::color() const
{
	return _color;
}

const float* SpriteGeometry::data() const
{
	return _data;
}

unsigned SpriteGeometry::data_size() const
{
	return s_geometry_size;
}

bool SpriteGeometry::has_changed() const
{
	return _has_changed;
}

void SpriteGeometry::reset_has_changed()
{
	_has_changed = false;
}
	
const Rect& SpriteGeometry::rect() const
{
	return _rect;
}

void SpriteGeometry::set_color(const Vector4& color)
{
	if (_color == color)
		return;

	_color = color;
	update_geometry(_color, _data, _rect, *_texture);
	_has_changed = true;
}

void SpriteGeometry::set_rect(const Rect& rect)
{
	_rect = rect;
	update_geometry(_color, _data, _rect, *_texture);
	_has_changed = true;
}

const Texture* SpriteGeometry::texture() const
{
	return _texture;
}


////////////////////////////////
// Implementation.

namespace
{

void update_geometry(const Color& color, void* data, const Rect& rect, const Texture& texture)
{	
	auto w = (float)rect.size.x;
	auto h = (float)rect.size.y;

	auto tex_w = (float)texture.image->resolution.x;
	auto tex_h = (float)texture.image->resolution.y;

	auto u_min = rect.position.x / tex_w;
	auto v_min = rect.position.y / tex_h;
	auto u_max = (rect.position.x + w) / tex_w;
	auto v_max = (rect.position.y + h) / tex_h;

	float geometry[54] = {
		0.0f, 0.0f, 0.0f,
		u_min, v_min,
		color.x, color.y, color.z, color.w,
		w, 0.0f, 0.0f,
		u_max, v_min,
		color.x, color.y, color.z, color.w,
		0.0f, h, 0.0f,
		u_min, v_max,
		color.x, color.y, color.z, color.w,

		w, 0.0f, 0.0f,
		u_max, v_min,
		color.x, color.y, color.z, color.w,
		w, h, 0.0f,
		u_max, v_max,
		color.x, color.y, color.z, color.w,
		0.0f, h, 0.0f,
		u_min, v_max,
		color.x, color.y, color.z, color.w
	};

	memcpy((void*)data, geometry, SpriteGeometry::s_geometry_size);
}

} // implementation

} // namespace bowtie
