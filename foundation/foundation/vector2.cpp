#include "vector2.h"

namespace bowtie
{	
namespace vector2
{

Vector2 create(float x, float y)
{
	Vector2 v = {x, y};
	return v;
}

bool equals(const Vector2* v1, const Vector2* v2)
{
	return v1->x == v2->x && v1->y == v2->y;
}

void inc(Vector2* to, const Vector2* from)
{
	to->x += from->x;
	to->y += from->y;
}

Vector2 add(const Vector2* v1, const Vector2* v2)
{
	Vector2 v1v2 = {
		v1->x + v2->x,
		v1->y + v2->y,
	};
	return v1v2;
}

void dec(Vector2* to, const Vector2* from)
{
	to->x -= from->x;
	to->y -= from->y;
}

Vector2 sub(const Vector2* v1, const Vector2* v2)
{
	Vector2 v1v2 = {
		v1->x - v2->x,
		v1->y - v2->y
	};

	return v1v2;
}

void scale(Vector2* v1, float scalar)
{
	v1->x *= scalar;
	v1->y *= scalar;
}

Vector2 mul(const Vector2* v1, float scalar)
{
	Vector2 v1s = {
		v1->x * scalar,
		v1->y * scalar
	};

	return v1s;
}

void scale(Vector2* v1, const Vector2* v2)
{
	v1->x *= v2->x;
	v1->y *= v2->y;
}

Vector2 mul(const Vector2* v1, const Vector2* v2)
{
	Vector2 v1v2 = {
		v1->x * v2->x,
		v1->y * v2->y
	};

	return v1v2;
}

float dot(const Vector2* v1, const Vector2* v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

} // namespace Vector2
} // namespace bowtie
