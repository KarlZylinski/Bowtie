#pragma once

namespace bowtie
{

struct Vector2
{
	float x, y;
};

namespace vector2
{
	Vector2 create(float x, float y);
	bool equals(const Vector2* v1, const Vector2* v2);
	void inc(Vector2* to, const Vector2* from);
	Vector2 add(const Vector2* v1, const Vector2* v2);
	void dec(Vector2* to, const Vector2* from);
	Vector2 sub(const Vector2* v1, const Vector2* v2);
	void scale(Vector2* v1, float scalar);
	Vector2 mul(const Vector2* v1, float scalar);
	void scale(Vector2* v1, const Vector2* v2);
	Vector2 mul(const Vector2* v1, const Vector2* v2);
	float dot(const Vector2* v1, const Vector2* v2);
}
}