#include "vector4.h"

namespace bowtie
{    
namespace vector4
{

Vector4 create(real32 x, real32 y, real32 z, real32 w)
{
    Vector4 v = {x, y, z, w};
    return v;
}

bool equals(const Vector4* v1, const Vector4* v2)
{
    return v1->x == v2->x && v1->y == v2->y && v1->z == v2->z && v1->w == v2->w;
}

void inc(Vector4* to, const Vector4* from)
{
    to->x += from->x;
    to->y += from->y;
    to->z += from->z;
    to->w += from->w;
}

Vector4 add(const Vector4* v1, const Vector4* v2)
{
    Vector4 v1v2 = {
        v1->x + v2->x,
        v1->y + v2->y,
        v1->z + v2->z,
        v1->w + v2->w,
    };
    return v1v2;
}

void dec(Vector4* to, const Vector4* from)
{
    to->x -= from->x;
    to->y -= from->y;
    to->z -= from->z;
    to->w -= from->w;
}

Vector4 sub(const Vector4* v1, const Vector4* v2)
{
    Vector4 v1v2 = {
        v1->x - v2->x,
        v1->y - v2->y,
        v1->z - v2->z,
        v1->w - v2->w,
    };

    return v1v2;
}

void scale(Vector4* v1, real32 scalar)
{
    v1->x *= scalar;
    v1->y *= scalar;
    v1->z *= scalar;
    v1->w *= scalar;
}

Vector4 mul(const Vector4* v1, real32 scalar)
{
    Vector4 v1s = {
        v1->x * scalar,
        v1->y * scalar,
        v1->z * scalar,
        v1->w * scalar
    };

    return v1s;
}

void scale(Vector4* v1, const Vector4* v2)
{
    v1->x *= v2->x;
    v1->y *= v2->y;
    v1->z *= v2->z;
    v1->w *= v2->w;
}

Vector4 mul(const Vector4* v1, const Vector4* v2)
{
    Vector4 v1v2 = {
        v1->x * v2->x,
        v1->y * v2->y,
        v1->z * v2->z,
        v1->w * v2->w
    };

    return v1v2;
}

real32 dot(const Vector4* v1, const Vector4* v2)
{
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

} // namespace vector4
} // namespace bowtie
