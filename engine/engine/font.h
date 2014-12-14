#pragma once

#include "rect.h"
#include <foundation/vector2u.h>

namespace bowtie
{

struct Texture;
struct Font
{
    uint32 columns;
    uint32 rows;
    const Texture* texture;
};

namespace font
{
    Vector2u char_size(const Font* font);
    Rect char_uv(const Font* font, char c);
}

}