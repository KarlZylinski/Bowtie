#pragma once

namespace bowtie
{

struct Vector2u
{
	unsigned x, y;
};

namespace vector2u
{
	Vector2u create(unsigned x, unsigned y);
	bool equals(const Vector2u* v1, const Vector2u* v2);
	void inc(Vector2u* to, const Vector2u* from);
	Vector2u add(const Vector2u* v1, const Vector2u* v2);
	void dec(Vector2u* to, const Vector2u* from);
	Vector2u sub(const Vector2u* v1, const Vector2u* v2);
	void scale(Vector2u* v1, unsigned scalar);
	Vector2u mul(const Vector2u* v1, unsigned scalar);
	void scale(Vector2u* v1, const Vector2u* v2);
	Vector2u mul(const Vector2u* v1, const Vector2u* v2);
	unsigned dot(const Vector2u* v1, const Vector2u* v2);
}
}
