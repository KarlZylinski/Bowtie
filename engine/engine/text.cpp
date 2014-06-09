#include "text.h"
#include "font.h"

#include <foundation/memory.h>
#include <foundation/string_utils.h>

namespace bowtie
{

Text::Text(const Font& font, Allocator& allocator) : _font(font), _allocator(allocator), _geometry(0), _text(0) {}

Text::~Text()
{
	_allocator.deallocate(_geometry);
	_allocator.deallocate(_text);
}

void Text::set_text(const char* text)
{
	_allocator.deallocate(_text);
	auto text_len = strlen32(text) + 1;
	_text = (char*)_allocator.allocate(text_len);
	memcpy(_text, text, text_len);
	
	update_geometry();
	set_geometry_changed();
}

void Text::update_geometry()
{
	auto text_len = strlen32(_text);
	auto size_of_char_geometry = sizeof(float) * 5 * 6;
	_allocator.deallocate(_geometry);
	_geometry = (float*)_allocator.allocate((unsigned)size_of_char_geometry * text_len);
	
	auto size = _font.char_size();
	auto pos = Vector2u();
	for (unsigned i = 0; i < text_len; ++i)
	{
		char c = _text[i];
		auto uv = _font.char_uv(c);

		auto x = (float)pos.x;
		auto y = (float)pos.y;
		auto w = pos.x + (float)size.x;
		auto h = pos.y + (float)size.y;

		auto u_min = uv.position.x;
		auto v_min = uv.position.y;
		auto u_max = uv.size.x;
		auto v_max = uv.size.y;

		float geometry[30] = {
			x, y, 0.0f,
			u_min, v_min,
			w, y, 0.0f,
			u_max, v_min,
			x, h, 0.0f,
			u_min, v_max,

			w, y, 0.0f,
			u_max, v_min,
			w, h, 0.0f,
			u_max, v_max,
			x, h, 0.0f,
			u_min, v_max
		};

		pos.x += size.x;

		memcpy(memory::pointer_add(_geometry, i * (unsigned)size_of_char_geometry), geometry, size_of_char_geometry);
	}
}

const float* Text::geometry_data() const
{
	return _geometry;
}

unsigned Text::geometry_size() const
{
	auto text_len = strlen32(_text);
	auto size_of_char_geometry = sizeof(float) * 5 * 6;

	return text_len * (unsigned)size_of_char_geometry;
}

const Texture* Text::texture() const
{
	return &_font.texture();
}

}
