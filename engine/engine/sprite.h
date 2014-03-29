#pragma once

#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/matrix4.h>

#include "resource_handle.h"

namespace bowtie
{

struct Image;
class Sprite
{
public:
	Sprite();
	Sprite(const Image& image);
	void set_position(const Vector2& position);
	const Vector2& position() const;

	void set_size(const Vector2u& size);
	const Vector2u& size() const;

	Matrix4 model_matrix() const;
	const Image* image() const;

	void set_render_handle(ResourceHandle handle);
	ResourceHandle render_handle() const;

private:
	ResourceHandle _render_handle;
	const Image* _image;
	Vector2 _position;
	Vector2u _size;
};

}