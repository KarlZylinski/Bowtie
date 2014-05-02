#include "sprite.h"
#include "texture.h"

namespace bowtie
{

Sprite::Sprite(const Texture& texture) : _texture(&texture), _size(texture.resolution)
{
}

void Sprite::set_position(const Vector2& position)
{
	_position = position;
}

const Vector2& Sprite::position() const
{
	return _position;
}

void Sprite::set_size(const Vector2u& size)
{
	_size = size;
}

const Vector2u& Sprite::size() const
{
	return _size;
}

Matrix4 Sprite::model_matrix() const
{
	auto m = Matrix4();

	m[0][0] = _size.x / float(_texture->resolution.x);
	m[1][1] = _size.y / float(_texture->resolution.y);
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
	assert(_render_handle.type != ResourceHandle::NotInitialized && "Trying to get non-initialized sprite render handle.");
	
	return _render_handle;
}

} // namespace bowtie
