#pragma once

namespace bowtie
{

struct Vector4
{
	union { real32 x, r; };
	union { real32 y, g; };
	union { real32 z, b; };
	union { real32 w, a; };
};

typedef Vector4 Color;

namespace vector4
{
	Vector4 create(real32 x, real32 y, real32 z, real32 w);
	bool equals(const Vector4* v1, const Vector4* v2);
	void inc(Vector4* to, const Vector4* from);
	Vector4 add(const Vector4* v1, const Vector4* v2);
	void dec(Vector4* to, const Vector4* from);
	Vector4 sub(const Vector4* v1, const Vector4* v2);
	void scale(Vector4* v1, real32 scalar);
	Vector4 mul(const Vector4* v1, real32 scalar);
	void scale(Vector4* v1, const Vector4* v2);
	Vector4 mul(const Vector4* v1, const Vector4* v2);
	real32 dot(const Vector4* v1, const Vector4* v2);
}
}
