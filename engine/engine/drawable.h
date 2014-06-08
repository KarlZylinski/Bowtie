#pragma once

#include <foundation/matrix4.h>
#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include "resource_handle.h"

namespace bowtie
{

struct Texture;
class Drawable
{
public:
	virtual ~Drawable();

	Matrix4 model_matrix() const;

	void set_render_handle(ResourceHandle handle);
	ResourceHandle render_handle() const;
	void reset_state_changed();
	bool state_changed() const;

	void reset_geometry_changed();
	bool geometry_changed() const;
	void set_geometry(ResourceHandle geometry);
	ResourceHandle geometry() const;
	virtual const float* geometry_data() const = 0;
	virtual unsigned geometry_size() const = 0;
	
	void set_shader(ResourceHandle shader);
	ResourceHandle shader() const;

	virtual const Texture* texture() const { return nullptr; }

	void set_position(const Vector2& position);
	const Vector2& position() const;

	void set_size(const Vector2u& size);
	const Vector2u& size() const;

protected:
	void set_render_state_changed() { _render_state_changed = true; }
	void set_geometry_changed() { _geometry_changed = true; }

private:
	ResourceHandle _render_handle;
	ResourceHandle _shader;
	ResourceHandle _geometry;
	Vector2 _position;
	Vector2u _size;
	bool _render_state_changed;
	bool _geometry_changed;
};

}