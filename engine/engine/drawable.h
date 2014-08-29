#pragma once

#include <foundation/matrix4.h>
#include <foundation/vector2.h>
#include <foundation/vector2i.h>
#include <foundation/vector4.h>
#include "resource_handle.h"

namespace bowtie
{

class Allocator;
class IDrawableGeometry;
class Drawable
{
public:
	Drawable(Allocator& allocator, IDrawableGeometry& geometry);
	Drawable(const Drawable& other);
	~Drawable();
	
	const Color& color() const;
	IDrawableGeometry& geometry();
	const IDrawableGeometry& geometry() const;
	bool geometry_changed() const;
	ResourceHandle geometry_handle() const;
	ResourceHandle material() const;
	Matrix4 model_matrix() const;
	const Vector2i& pivot() const;
	const Vector2& position() const;
	ResourceHandle render_handle() const;
	void reset_geometry_changed();
	void reset_state_changed();
	float rotation() const;
	void set_color(const Color& color);
	void set_geometry_handle(ResourceHandle handle);
	void set_material(ResourceHandle material);
	void set_pivot(const Vector2i& position);
	void set_position(const Vector2& position);
	void set_render_handle(ResourceHandle handle);
	void set_rotation(float rotation);
	bool state_changed() const;

private:
	Allocator& _allocator;
	ResourceHandle _geometry_handle;
	IDrawableGeometry& _geometry;
	Vector2i _pivot;
	Vector2 _position;
	ResourceHandle _render_handle;
	bool _render_state_changed;
	float _rotation;
	ResourceHandle _material;

	Drawable& operator=(const Drawable&);
};

}