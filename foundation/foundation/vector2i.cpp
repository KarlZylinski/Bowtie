#include "vector2i.h"

namespace bowtie
{	
namespace vector2i
{

Vector2i create(int32 x, int32 y)
{
	Vector2i v = {x, y};
	return v;
}

bool equals(const Vector2i* v1, const Vector2i* v2)
{
	return v1->x == v2->x && v1->y == v2->y;
}

void inc(Vector2i* to, const Vector2i* from)
{
	to->x += from->x;
	to->y += from->y;
}

Vector2i add(const Vector2i* v1, const Vector2i* v2)
{
	Vector2i v1v2 = {
		v1->x + v2->x,
		v1->y + v2->y,
	};
	return v1v2;
}

void dec(Vector2i* to, const Vector2i* from)
{
	to->x -= from->x;
	to->y -= from->y;
}

Vector2i sub(const Vector2i* v1, const Vector2i* v2)
{
	Vector2i v1v2 = {
		v1->x - v2->x,
		v1->y - v2->y
	};

	return v1v2;
}

void scale(Vector2i* v1, int32 scalar)
{
	v1->x *= scalar;
	v1->y *= scalar;
}

Vector2i mul(const Vector2i* v1, int32 scalar)
{
	Vector2i v1s = {
		v1->x * scalar,
		v1->y * scalar
	};

	return v1s;
}

void scale(Vector2i* v1, const Vector2i* v2)
{
	v1->x *= v2->x;
	v1->y *= v2->y;
}

Vector2i mul(const Vector2i* v1, const Vector2i* v2)
{
	Vector2i v1v2 = {
		v1->x * v2->x,
		v1->y * v2->y
	};

	return v1v2;
}

int32 dot(const Vector2i* v1, const Vector2i* v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

} // namespace Vector2i
} // namespace bowtie
