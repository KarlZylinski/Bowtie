#include "text_geometry.h"
#include "font.h"

#include <foundation/memory.h>
#include <foundation/string_utils.h>
#include <foundation/vector4.h>

namespace bowtie
{

namespace
{
float* update_geometry(Allocator& allocator, const Color& color, const Font& font, const char* text);
}


////////////////////////////////
// Public interface.

TextGeometry::TextGeometry(const Font& font, Allocator& allocator) : _allocator(allocator), _color(1, 1, 1, 1), _font(font), _data(0), _has_changed(false), _text(0)
{
}

TextGeometry::~TextGeometry()
{
	_allocator.deallocate(_data);
	_allocator.deallocate(_text);
}

const IDrawableGeometry& TextGeometry::clone(Allocator& allocator) const
{
	return *allocator.construct<TextGeometry>(*this);
}

IDrawableGeometry& TextGeometry::clone(Allocator& allocator)
{
	return *allocator.construct<TextGeometry>(*this);
}

const Vector4& TextGeometry::color() const
{
	return _color;
}

const float* TextGeometry::data() const
{
	return _data;
}

bool TextGeometry::has_changed() const
{
	return _has_changed;
}

void TextGeometry::reset_has_changed()
{
	_has_changed = false;
}

void TextGeometry::set_color(const Vector4& color)
{
	_color = color;
}

void TextGeometry::set_text(const char* text)
{
	_allocator.deallocate(_text);
	_text = _allocator.copy_string(text);
	_allocator.deallocate(_data);
	_data = update_geometry(_allocator, _color, _font, _text);
	_has_changed = true;
}

unsigned TextGeometry::size() const
{
	auto text_len = strlen32(_text);
	auto size_of_char_geometry = sizeof(float) * 9 * 6;
	return text_len * (unsigned)size_of_char_geometry;
}

const Texture* TextGeometry::texture() const
{
	return &_font.texture();
}


////////////////////////////////
// Implementation.

namespace
{

float* update_geometry(Allocator& allocator, const Color& color, const Font& font, const char* text)
{
	auto text_len = strlen32(text);
	auto size_of_char_geometry = sizeof(float) * 9 * 6;

	if (text_len == 0)
		return nullptr;

	auto data = (float*)allocator.allocate((unsigned)size_of_char_geometry * text_len);
	
	auto size = font.char_size();
	auto pos = Vector2u();
	for (unsigned i = 0; i < text_len; ++i)
	{
		char c = text[i];
		auto uv = font.char_uv(c);

		auto x = (float)pos.x;
		auto y = (float)pos.y;
		auto w = pos.x + (float)size.x;
		auto h = pos.y + (float)size.y;

		auto u_min = uv.position.x;
		auto v_min = uv.position.y;
		auto u_max = uv.size.x;
		auto v_max = uv.size.y;

		float geometry[54] = {
			x, y, 0.0f,
			u_min, v_min,
			color.x, color.y, color.z, color.w,
			w, y, 0.0f,
			u_max, v_min,
			color.x, color.y, color.z, color.w,
			x, h, 0.0f,
			u_min, v_max,
			color.x, color.y, color.z, color.w,

			w, y, 0.0f,
			u_max, v_min,
			color.x, color.y, color.z, color.w,
			w, h, 0.0f,
			u_max, v_max,
			color.x, color.y, color.z, color.w,
			x, h, 0.0f,
			u_min, v_max,
			color.x, color.y, color.z, color.w,
		};

		pos.x += size.x;
		memcpy(memory::pointer_add(data, i * (unsigned)size_of_char_geometry), geometry, size_of_char_geometry);
	}

	return data;
}

}

}
