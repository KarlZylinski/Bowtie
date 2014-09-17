#pragma once

#include <foundation/collection_types.h>
#include <foundation/matrix4.h>
#include <foundation/vector2.h>
#include <foundation/vector2i.h>
#include <foundation/vector4.h>
#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
class IDrawableGeometry;
struct Material;
class Drawable
{
public:
	Drawable(Allocator& allocator, IDrawableGeometry& geometry, Material* material);
	Drawable(const Drawable& other);
	~Drawable();
	
	void add_child(Drawable* child);
	void remove_child(Drawable* child);
	void set_parent(Drawable* parent);
	Drawable* parent() const;
	const Color& color() const;
	float depth() const;
	IDrawableGeometry& geometry();
	const IDrawableGeometry& geometry() const;
	bool geometry_changed() const;
	RenderResourceHandle geometry_handle() const;
	Material* material() const;
	Matrix4 model_matrix() const;
	const Vector2i& pivot() const;
	const Vector2& position() const;
	RenderResourceHandle render_handle() const;
	void set_state_changed();
	void reset_geometry_changed();
	void reset_state_changed();
	float rotation() const;
	void set_color(const Color& color);
	void set_depth(float depth);
	void set_geometry_handle(RenderResourceHandle handle);
	void set_material(Material* material);
	void set_pivot(const Vector2i& position);
	void set_position(const Vector2& position);
	void set_render_handle(RenderResourceHandle handle);
	void set_rotation(float rotation);
	bool state_changed() const;

private:
	Allocator& _allocator;
	Array<Drawable*> _children;
	float _depth;
	RenderResourceHandle _geometry_handle;
	IDrawableGeometry& _geometry;
	Vector2i _pivot;
	Drawable* _parent;
	Vector2 _position;
	RenderResourceHandle _render_handle;
	bool _render_state_changed;
	float _rotation;
	Material* _material;

	Drawable& operator=(const Drawable&);
};

}