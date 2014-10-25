#pragma once

#include <foundation/vector4.h>
#include "idrawable_geometry.h"

namespace bowtie
{

struct Font;
class Allocator;
class TextGeometry : public IDrawableGeometry
{
public:
	TextGeometry(const Font& font, Allocator& allocator);
	~TextGeometry();
	
	const IDrawableGeometry& clone(Allocator& allocator) const;
	IDrawableGeometry& clone(Allocator& allocator);
	const Vector4& color() const;
	const float* data() const;
	unsigned data_size() const;
	bool has_changed() const;
	void reset_has_changed();
	void set_color(const Vector4& color);
	void set_text(const char* text);
	const Texture* texture() const;

private:
	Allocator& _allocator;
	bool _has_changed;
	Color _color;
	const Font& _font;
	float* _data;
	char* _text;

	TextGeometry& operator=(const TextGeometry&);
};

}