#include "drawable.h"

namespace bowtie
{

Drawable::~Drawable()
{
}

void Drawable::set_render_handle(ResourceHandle handle)
{
	assert(_render_handle.type == ResourceHandle::NotInitialized && "Trying to reset already initliaized drawable render handle.");

	_render_handle = handle;
}

ResourceHandle Drawable::render_handle() const
{
	return _render_handle;
}

void Drawable::set_shader(ResourceHandle shader)
{
	_shader = shader;
	set_render_state_changed();
}

ResourceHandle Drawable::shader() const
{
	return _shader;
}

void Drawable::reset_state_changed()
{
	_render_state_changed = false;
}

bool Drawable::state_changed() const
{
	return _render_state_changed;
}

ResourceHandle Drawable::geometry() const
{
	return _geometry;
}


void Drawable::set_position(const Vector2& position)
{
	_position = position;
	set_render_state_changed();
}

const Vector2& Drawable::position() const
{
	return _position;
}

void Drawable::set_size(const Vector2u& size)
{
	_size = size;
	_render_state_changed = true;
}

const Vector2u& Drawable::size() const
{
	return _size;
}

Matrix4 Drawable::model_matrix() const
{
	auto m = Matrix4();

	m[3][0] = _position.x;
	m[3][1] = _position.y;

	return m;
}

void Drawable::set_geometry(ResourceHandle geometry)
{
	assert (_geometry.type == ResourceHandle::NotInitialized && "Drawable already has geometry set");
	_geometry = geometry;
}

void Drawable::reset_geometry_changed()
{
	_geometry_changed = false;
}

bool Drawable::geometry_changed() const
{
	return _geometry_changed;
}

}
