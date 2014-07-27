#include "font.h"
#include "texture.h"

namespace bowtie
{

Font::Font(unsigned columns, unsigned rows, const Texture& texture) : _columns(columns), _rows(rows), _texture(texture)
{
}

Vector2u Font::char_size() const
{
	return Vector2u(_texture.resolution.x / _columns, _texture.resolution.y / _rows);	
}

Rect Font::char_uv(char c) const
{
	auto x = c % _columns;
	auto y = (c / _columns) % _rows;
	auto char_coord = Vector2u(x, y);
	auto char_size = this->char_size();
	auto char_pos = char_coord * char_size;
	auto char_uv_min = Vector2(char_pos.x / (float)_texture.resolution.x, char_pos.y / (float)_texture.resolution.y);
	auto char_uv_max = char_uv_min + Vector2(char_size.x / (float)_texture.resolution.x, char_size.y / (float)_texture.resolution.y);

	return Rect(char_uv_min, char_uv_max);
}

const Texture& Font::texture() const
{
	return _texture;
}

}
