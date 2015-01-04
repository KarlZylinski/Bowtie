#include "rect.h"

namespace bowtie
{
namespace rect
{

void init(Rect* r, const Vector2* position, const Vector2* size)
{
    r->position = *position;
    r->size = *size;
}

Rect create(const Vector2* position, const Vector2* size)
{
    Rect r;
    init(&r, position, size);
    return r;
}

}
}