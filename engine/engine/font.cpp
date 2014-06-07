#include "font.h"

namespace bowtie
{

Font::Font(const Texture& texture) : _texture(texture) {}

const Texture& Font::texture() const
{
	return _texture;
}

}
