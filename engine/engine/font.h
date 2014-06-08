#pragma once

#include "rect.h"
#include <foundation/vector2u.h>

namespace bowtie
{

struct Texture;
class Font
{
public:
	Font(const Texture& texture, unsigned columns, unsigned rows);

	const Texture& texture() const;
	Vector2u char_size() const;
	Rect char_uv(char c) const;

private:
	const Texture& _texture;
	unsigned _columns;
	unsigned _rows;

	Font(const Font&);
	Font& operator=(const Font&);
};

}