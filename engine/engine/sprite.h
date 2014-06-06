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
	
	void set_shader(ResourceHandle shader);
	ResourceHandle shader() const;

	void reset_state_changed();
	bool state_changed() const;

private:
	ResourceHandle _render_handle;
	ResourceHandle _shader;
	const Texture* _texture;
	Vector2 _position;
	Vector2u _size;
	bool _render_state_changed;
};

}