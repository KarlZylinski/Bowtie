#pragma once

#include "rect.h"
#include <foundation/vector2u.h>

namespace bowtie
{

struct Texture;
class Font
{
public:
	Font(unsigned columns, unsigned rows, const Texture& texture);
	
	Vector2u char_size() const;
	Rect char_uv(char c) const;
	const Texture& texture() const;

private:
	unsigned _columns;
	unsigned _rows;
	const Texture& _texture;

	Font(const Font&);
	Font& operator=(const Font&);
};

}