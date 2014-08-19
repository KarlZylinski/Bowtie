#pragma once

namespace bowtie
{

struct Vector2i
{
	Vector2i(int x, int y) : x(x), y(y) {}
	Vector2i() : x(0), y(0) {}
		
	inline void operator+=(const Vector2i& other)
	{
		x += other.x;
		y += other.y;
	}

	inline void operator-=(const Vector2i& other)
	{
		x -= other.x;
		y -= other.y;
	}

	inline void operator*=(const Vector2i& other)
	{
		x *= other.x;
		y *= other.y;
	}
	
	inline void operator*=(int s)
	{
		x *= s;
		y *= s;
	}

	int x, y;
};

inline Vector2i operator+(const Vector2i& v1, const Vector2i& v2)
{
	Vector2i new_vec = v1;
	new_vec += v2;
	return new_vec;
}

inline Vector2i operator-(const Vector2i& v1, const Vector2i& v2)
{
	Vector2i new_vec = v1;
	new_vec -= v2;
	return new_vec;
}

inline Vector2i operator*(const Vector2i& v1, const Vector2i& v2)
{
	Vector2i new_vec = v1;
	new_vec *= v2;
	return new_vec;
}

inline Vector2i operator*(const Vector2i& v, int s)
{
	Vector2i new_vec = v;
	new_vec *= s;
	return new_vec;
}

inline Vector2i operator*(int s, const Vector2i& v)
{
	Vector2i new_vec = v;
	new_vec *= s;
	return new_vec;
}

} // namespace bowtie