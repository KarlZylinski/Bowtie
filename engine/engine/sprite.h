#pragma once

#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/matrix4.h>

#include "resource_handle.h"

namespace bowtie
{

struct Texture;
class Sprite
{
public:
	Sprite(const Texture& texture);
	void set_position(const Vector2& position);
	const Vector2& position() const;

	void set_size(const Vector2u& size);
	const Vector2u& size() const;

	Matrix4 model_matrix() const;
	const Texture* texture() const;

	void set_render_handle(ResourceHandle handle);
	ResourceHandle render_handle() const;

private:
	ResourceHandle _render_handle;
	const Texture* _texture;
	Vector2 _position;
	Vector2u _size;
};

}