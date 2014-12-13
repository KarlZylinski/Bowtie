#pragma once

namespace bowtie
{

struct Vector4
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };
	union { float w, a; };
};

typedef Vector4 Color;

namespace vector4
{
	Vector4 create(float x, float y, float z, float w);
	bool equals(const Vector4* v1, const Vector4* v2);
	void inc(Vector4* to, const Vector4* from);
	Vector4 add(const Vector4* v1, const Vector4* v2);
	void dec(Vector4* to, const Vector4* from);
	Vector4 sub(const Vector4* v1, const Vector4* v2);
	void scale(Vector4* v1, float scalar);
	Vector4 mul(const Vector4* v1, float scalar);
	void scale(Vector4* v1, const Vector4* v2);
	Vector4 mul(const Vector4* v1, const Vector4* v2);
	float dot(const Vector4* v1, const Vector4* v2);
}
}
