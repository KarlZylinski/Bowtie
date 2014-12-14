#pragma once

namespace bowtie
{

struct Vector2i
{
    int32 x, y;
};

namespace vector2i
{
    Vector2i create(int32 x, int32 y);
    bool equals(const Vector2i* v1, const Vector2i* v2);
    void inc(Vector2i* to, const Vector2i* from);
    Vector2i add(const Vector2i* v1, const Vector2i* v2);
    void dec(Vector2i* to, const Vector2i* from);
    Vector2i sub(const Vector2i* v1, const Vector2i* v2);
    void scale(Vector2i* v1, int32 scalar);
    Vector2i mul(const Vector2i* v1, int32 scalar);
    void scale(Vector2i* v1, const Vector2i* v2);
    Vector2i mul(const Vector2i* v1, const Vector2i* v2);
    int32 dot(const Vector2i* v1, const Vector2i* v2);
}
}
