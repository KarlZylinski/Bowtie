#pragma once

#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/matrix4.h>

#include "resource_handle.h"

namespace bowtie
{

struct Rect
{
	Rect(const Vector2u& position, const Vector2u& size) : position(position), size(size)
	{
	}

	Rect() {}

	Vector2u position;
	Vector2u size;
};

struct Texture;
class Allocator;
class Sprite
{
public:
	static const unsigned geometry_size = sizeof(float) * 5 * 6;

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

	void set_rect(const Rect& rect);
	const Rect& rect() const;

	ResourceHandle geometry() const;
	void set_geometry(ResourceHandle geometry);
	void reset_geometry_changed();
	bool geometry_changed() const;

	const float* geometry_data() const;

private:
	ResourceHandle _render_handle;
	ResourceHandle _shader;
	ResourceHandle _geometry;
	float _geometry_data[geometry_size];
	const Texture* _texture;
	Vector2 _position;
	Vector2u _size;
	Rect _rect;
	bool _render_state_changed;
	bool _geometry_changed;

	void update_geometry();

	Sprite(const Sprite&);
	Sprite& operator=(const Sprite&);
};

}