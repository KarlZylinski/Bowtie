#pragma once
#include <cstring>
#include "vector4.h"

namespace bowtie
{

struct Matrix4
{
    Vector4 x, y, z, w;
};

namespace matrix4
{
    Matrix4 indentity();
    Matrix4 mul(const Matrix4* m1, const Matrix4* m2);
    Vector4 mul(const Matrix4* m, const Vector4* v);
}

} // namespace bowtie