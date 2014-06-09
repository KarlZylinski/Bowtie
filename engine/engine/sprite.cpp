#include "sprite.h"
#include "texture.h"
#include <foundation/memory.h>

namespace bowtie
{

Sprite::Sprite(const Texture& texture) : _texture(&texture)
{
	_rect = Rect(Vector2(), Vector2((float)_texture->resolution.x, (float)_texture->resolution.y));
	update_geometry();
}

void Sprite::update_geometry()
{	
	auto w = (float)_rect.size.x;
	auto h = (float)_rect.size.y;

	auto tex_w = (float)_texture->resolution.x;
	auto tex_h = (float)_texture->resolution.y;

	auto u_min = _rect.position.x / tex_w;
	auto v_min = _rect.position.y / tex_h;
	auto u_max = (_rect.position.x + w) / tex_w;
	auto v_max = (_rect.position.y + h) / tex_h;

	auto color = Drawable::color();

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

	memcpy((void*)_geometry_data, geometry, s_geometry_size);
	set_geometry_changed();
}

const Texture* Sprite::texture() const
{
	return _texture;
}

void Sprite::set_rect(const Rect& rect)
{
	_rect = rect;
	update_geometry();
}

const Rect& Sprite::rect() const
{
	return _rect;
}

const float* Sprite::geometry_data() const
{
	return _geometry_data;
}

unsigned Sprite::geometry_size() const
{
	return s_geometry_size;
}

} // namespace bowtie
