#pragma once

namespace bowtie
{

struct Vector2u
{
    uint32 x, y;
};

namespace vector2u
{
    Vector2u create(uint32 x, uint32 y);
    bool equals(const Vector2u* v1, const Vector2u* v2);
    void inc(Vector2u* to, const Vector2u* from);
    Vector2u add(const Vector2u* v1, const Vector2u* v2);
    void dec(Vector2u* to, const Vector2u* from);
    Vector2u sub(const Vector2u* v1, const Vector2u* v2);
    void scale(Vector2u* v1, uint32 scalar);
    Vector2u mul(const Vector2u* v1, uint32 scalar);
    void scale(Vector2u* v1, const Vector2u* v2);
    Vector2u mul(const Vector2u* v1, const Vector2u* v2);
    uint32 dot(const Vector2u* v1, const Vector2u* v2);
}
}
