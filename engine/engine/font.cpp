#include "font.h"
#include "texture.h"

namespace bowtie
{

namespace font
{
	Vector2u char_size(const Font* font)
	{
		return vector2u::create(font->texture->image->resolution.x / font->columns, font->texture->image->resolution.y / font->rows);
	}

	Rect char_uv(const Font* font, char c)
	{
		auto x = c % font->columns;
		auto y = (c / font->columns) % font->rows;
		auto char_coord = vector2u::create(x, y);
		auto char_size = font::char_size(font);
		auto char_pos = vector2u::mul(&char_coord, &char_size);
		auto char_uv_min = vector2::create(char_pos.x / (real32)font->texture->image->resolution.x, char_pos.y / (real32)font->texture->image->resolution.y);
		auto char_uv_max = vector2::add(&char_uv_min, &vector2::create(char_size.x / (real32)font->texture->image->resolution.x, char_size.y / (real32)font->texture->image->resolution.y));
		Rect rect;
		rect::init(&rect, &char_uv_min, &char_uv_max);
		return rect;
	}
}


}
