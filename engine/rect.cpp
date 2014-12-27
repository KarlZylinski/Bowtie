#include "rect.h"

namespace bowtie
{
namespace rect
{

void init(Rect* r, Vector2* position, Vector2* size)
{
    r->position = *position;
    r->size = *size;
}

}
}