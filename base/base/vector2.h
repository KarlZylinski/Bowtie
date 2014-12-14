#pragma once

namespace bowtie
{

struct Vector2
{
    real32 x, y;
};

namespace vector2
{
    Vector2 create(real32 x, real32 y);
    bool equals(const Vector2* v1, const Vector2* v2);
    void inc(Vector2* to, const Vector2* from);
    Vector2 add(const Vector2* v1, const Vector2* v2);
    void dec(Vector2* to, const Vector2* from);
    Vector2 sub(const Vector2* v1, const Vector2* v2);
    void scale(Vector2* v1, real32 scalar);
    Vector2 mul(const Vector2* v1, real32 scalar);
    void scale(Vector2* v1, const Vector2* v2);
    Vector2 mul(const Vector2* v1, const Vector2* v2);
    real32 dot(const Vector2* v1, const Vector2* v2);
}
}