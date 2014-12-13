#include "vector2u.h"

namespace bowtie
{	
namespace vector2u
{

Vector2u create(uint32 x, uint32 y)
{
	Vector2u v = {x, y};
	return v;
}

bool equals(const Vector2u* v1, const Vector2u* v2)
{
	return v1->x == v2->x && v1->y == v2->y;
}

void inc(Vector2u* to, const Vector2u* from)
{
	to->x += from->x;
	to->y += from->y;
}

Vector2u add(const Vector2u* v1, const Vector2u* v2)
{
	Vector2u v1v2 = {
		v1->x + v2->x,
		v1->y + v2->y,
	};
	return v1v2;
}

void dec(Vector2u* to, const Vector2u* from)
{
	to->x -= from->x;
	to->y -= from->y;
}

Vector2u sub(const Vector2u* v1, const Vector2u* v2)
{
	Vector2u v1v2 = {
		v1->x - v2->x,
		v1->y - v2->y
	};

	return v1v2;
}

void scale(Vector2u* v1, uint32 scalar)
{
	v1->x *= scalar;
	v1->y *= scalar;
}

Vector2u mul(const Vector2u* v1, uint32 scalar)
{
	Vector2u v1s = {
		v1->x * scalar,
		v1->y * scalar
	};

	return v1s;
}

void scale(Vector2u* v1, const Vector2u* v2)
{
	v1->x *= v2->x;
	v1->y *= v2->y;
}

Vector2u mul(const Vector2u* v1, const Vector2u* v2)
{
	Vector2u v1v2 = {
		v1->x * v2->x,
		v1->y * v2->y
	};

	return v1v2;
}

uint32 dot(const Vector2u* v1, const Vector2u* v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

} // namespace Vector2u
} // namespace bowtie
