#pragma once

#include <base/vector2.h>

namespace bowtie
{

struct Rect
{
    Vector2 position;
    Vector2 size;
};

namespace rect
{
    void init(Rect* rect, const Vector2* position, const Vector2* size);
    Rect create(const Vector2* position, const Vector2* size);
}

}