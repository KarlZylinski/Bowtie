#pragma once

#include "drawable.h"

namespace bowtie
{

class Font;
class Allocator;
class Text : public Drawable
{
public:
	Text(const Font& font, Allocator& allocator);
	~Text();

	void set_text(const char* text);
	virtual const float* geometry_data() const;
	virtual unsigned geometry_size() const;
	virtual const Texture* texture() const;

private:
	void update_geometry();

	Allocator& _allocator;
	const Font& _font;
	char* _text;
	float* _geometry;

	Text(const Text&);
	Text& operator=(const Text&);
};

}