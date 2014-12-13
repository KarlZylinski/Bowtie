#include "font.h"
#include "texture.h"

namespace bowtie
{

namespace font
{
	Vector2u char_size(const Font* font)
	{
		return Vector2u(font->texture->image->resolution.x / font->columns, font->texture->image->resolution.y / font->rows);
	}

	Rect char_uv(const Font* font, char c)
	{
		auto x = c % font->columns;
		auto y = (c / font->columns) % font->rows;
		auto char_coord = Vector2u(x, y);
		auto char_size = font::char_size(font);
		auto char_pos = char_coord * char_size;
		auto char_uv_min = Vector2(char_pos.x / (float)font->texture->image->resolution.x, char_pos.y / (float)font->texture->image->resolution.y);
		auto char_uv_max = char_uv_min + Vector2(char_size.x / (float)font->texture->image->resolution.x, char_size.y / (float)font->texture->image->resolution.y);
		Rect rect;
		rect::init(&rect, &char_uv_min, &char_uv_max);
		return rect;
	}
}


}
