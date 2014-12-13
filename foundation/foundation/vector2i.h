#pragma once

namespace bowtie
{

struct Vector2i
{
	int x, y;
};

namespace vector2i
{
	Vector2i create(int x, int y);
	bool equals(const Vector2i* v1, const Vector2i* v2);
	void inc(Vector2i* to, const Vector2i* from);
	Vector2i add(const Vector2i* v1, const Vector2i* v2);
	void dec(Vector2i* to, const Vector2i* from);
	Vector2i sub(const Vector2i* v1, const Vector2i* v2);
	void scale(Vector2i* v1, int scalar);
	Vector2i mul(const Vector2i* v1, int scalar);
	void scale(Vector2i* v1, const Vector2i* v2);
	Vector2i mul(const Vector2i* v1, const Vector2i* v2);
	int dot(const Vector2i* v1, const Vector2i* v2);
}
}
