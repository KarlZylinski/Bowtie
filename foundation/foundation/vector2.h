#pragma once

namespace bowtie
{
	struct Vector2
	{
		Vector2(float x, float y) : x(x), y(y) {}
		Vector2() : x(0), y(0) {}
		
		inline void operator+=(const Vector2& other)
		{
			x += other.x;
			y += other.y;
		}		

		inline void operator-=(const Vector2& other)
		{
			x -= other.x;
			y -= other.y;
		}

		inline void operator*=(float scalar)
		{
			x *= scalar;
			y *= scalar;
		}

		float x, y;
	};

	inline Vector2 operator-(const Vector2& v)
	{
		return Vector2(-v.x, -v.y);
	}

	inline Vector2 operator+(const Vector2& v1, const Vector2& v2)
	{
		Vector2 new_vec = v1;
		new_vec += v2;
		return new_vec;
	}

	inline Vector2 operator-(const Vector2& v1, const Vector2& v2)
	{
		Vector2 new_vec = v1;
		new_vec -= v2;
		return new_vec;
	}

	inline Vector2 operator*(const Vector2& v1, float scalar)
	{
		Vector2 new_vec = v1;
		new_vec *= scalar;
		return new_vec;
	}
} // namespace bowtie