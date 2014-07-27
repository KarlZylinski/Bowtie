#pragma once

#include <foundation/vector4.h>
#include "idrawable_geometry.h"
#include "rect.h"

namespace bowtie
{

class Allocator;
class SpriteGeometry : public IDrawableGeometry
{
public:
	static const unsigned s_geometry_size = sizeof(float) * 9 * 6;

	SpriteGeometry(const Texture& texture);

	const IDrawableGeometry& clone(Allocator& allocator) const;
	IDrawableGeometry& clone(Allocator& allocator);
	const Vector4& color() const;
	const float* data() const;
	bool has_changed() const;
	void reset_has_changed();
	const Rect& rect() const;
	void set_color(const Vector4& color);
	void set_rect(const Rect& rect);
	unsigned size() const;
	const Texture* texture() const;

private:
	float _data[s_geometry_size];
	Color _color;
	bool _has_changed;
	Rect _rect;
	const Texture* _texture;
};

}