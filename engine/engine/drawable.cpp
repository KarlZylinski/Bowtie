#include "drawable.h"
#include <foundation/memory.h>
#include "idrawable_geometry.h"
#include <cmath>

namespace bowtie
{

////////////////////////////////
// Public interface.

Drawable::Drawable(Allocator& allocator, IDrawableGeometry& geometry) : _allocator(allocator), _geometry(geometry), _render_state_changed(false), _rotation(0.7f)
{
}

Drawable::Drawable(const Drawable& other) : _allocator(other._allocator), _geometry(other._geometry.clone(_allocator)), _render_state_changed(false), _rotation(0.7f), _position(other._position)
{
}

Drawable::~Drawable()
{
	_allocator.destroy(&_geometry);
}

const Color& Drawable::color() const
{
	return _geometry.color();
}

IDrawableGeometry& Drawable::geometry()
{
	return _geometry;
}

const IDrawableGeometry& Drawable::geometry() const
{
	return _geometry;
}

bool Drawable::geometry_changed() const
{
	return _geometry.has_changed();
}

ResourceHandle Drawable::geometry_handle() const
{
	return _geometry_handle;
}

Matrix4 Drawable::model_matrix() const
{
	auto m = Matrix4();
	m[3][0] = _position.x;
	m[3][1] = _position.y;
	m[0][0] = cos(_rotation);
	m[1][0] = -sin(_rotation);
	m[0][1] = sin(_rotation);
	m[1][1] = cos(_rotation);
	return m;
}

const Vector2& Drawable::position() const
{
	return _position;
}

ResourceHandle Drawable::render_handle() const
{
	return _render_handle;
}

void Drawable::reset_geometry_changed()
{
	_geometry.reset_has_changed();
}

void Drawable::reset_state_changed()
{
	_render_state_changed = false;
}

float Drawable::rotation() const
{
	return _rotation;
}

void Drawable::set_color(const Color& color)
{
	_geometry.set_color(color);
}

void Drawable::set_geometry_handle(ResourceHandle handle)
{
	_geometry_handle = handle;
}

void Drawable::set_position(const Vector2& position)
{
	_position = position;
	_render_state_changed = true;
}

void Drawable::set_render_handle(ResourceHandle handle)
{
	assert(_render_handle.type == ResourceHandle::NotInitialized && "Trying to reset already initliaized drawable render handle.");
	_render_handle = handle;
}

void Drawable::set_rotation(float rotation)
{
	_rotation = rotation;
	_render_state_changed = true;
}

void Drawable::set_shader(ResourceHandle shader)
{
	_shader = shader;
	_render_state_changed = true;
}

ResourceHandle Drawable::shader() const
{
	return _shader;
}

bool Drawable::state_changed() const
{
	return _render_state_changed;
}

}
