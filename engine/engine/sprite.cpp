#include "sprite.h"
#include "image.h"

namespace bowtie
{

Sprite::Sprite() : _image(nullptr)
{
}

Sprite::Sprite(const Image& image) : _image(&image), _size(image.size)
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

	m[0][0] = _size.x / float(_image->size.x);
	m[1][1] = _size.y / float(_image->size.y);
	m[3][0] = _position.x;
	m[3][1] = _position.y;

	return m;
}

const Image* Sprite::image() const
{
	return _image;
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
