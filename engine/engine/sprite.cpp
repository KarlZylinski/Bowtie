#include "sprite.h"
#include "texture.h"
#include <foundation/memory.h>

namespace bowtie
{

Sprite::Sprite(const Texture& texture) : _texture(&texture), _size(texture.resolution), _render_state_changed(false)
{
	_rect = Rect(Vector2u(), _texture->resolution);
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

	float geometry[30] = {
	   0.0f, 0.0f, 0.0f,
	   u_min, v_min,
	   w, 0.0f, 0.0f,
	   u_max, v_min,
	   0.0f, h, 0.0f,
	   u_min, v_max,

	   w, 0.0f, 0.0f,
	   u_max, v_min,
	   w, h, 0.0f,
	   u_max, v_max,
	   0.0f, h, 0.0f,
	   u_min, v_max
	};

	memcpy((void*)_geometry_data, geometry, sizeof(float) * 5 * 6);
	_geometry_changed = true;
}

void Sprite::set_position(const Vector2& position)
{
	_position = position;
	_render_state_changed = true;
}

const Vector2& Sprite::position() const
{
	return _position;
}

void Sprite::set_size(const Vector2u& size)
{
	_size = size;
	_render_state_changed = true;
}

const Vector2u& Sprite::size() const
{
	return _size;
}

Matrix4 Sprite::model_matrix() const
{
	auto m = Matrix4();

	m[3][0] = _position.x;
	m[3][1] = _position.y;

	return m;
}

const Texture* Sprite::texture() const
{
	return _texture;
}

void Sprite::set_render_handle(ResourceHandle handle)
{
	assert(_render_handle.type == ResourceHandle::NotInitialized && "Trying to reset already initliaized sprite render handle.");

	_render_handle = handle;
}

ResourceHandle Sprite::render_handle() const
{
	return _render_handle;
}

void Sprite::set_shader(ResourceHandle shader)
{
	_shader = shader;
	_render_state_changed = true;
}

ResourceHandle Sprite::shader() const
{
	return _shader;
}

void Sprite::reset_state_changed()
{
	_render_state_changed = false;
}

bool Sprite::state_changed() const
{
	return _render_state_changed;
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

ResourceHandle Sprite::geometry() const
{
	return _geometry;
}

const float* Sprite::geometry_data() const
{
	return _geometry_data;
}

void Sprite::set_geometry(ResourceHandle geometry)
{
	assert (_geometry.type == ResourceHandle::NotInitialized && "Sprite already has geometry set");
	_geometry = geometry;
}

void Sprite::reset_geometry_changed()
{
	_geometry_changed = false;
}

bool Sprite::geometry_changed() const
{
	return _geometry_changed;
}

} // namespace bowtie
