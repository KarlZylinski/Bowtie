#pragma once

namespace bowtie
{
	struct Vector2u
	{
		Vector2u(unsigned x, unsigned y) : x(x), y(y) {}
		Vector2u() : x(0), y(0) {}
		
		inline void operator+=(const Vector2u& other)
		{
			x += other.x;
			y += other.y;
		}		

		inline void operator-=(const Vector2u& other)
		{
			x -= other.x;
			y -= other.y;
		}

		unsigned x, y;
	};

	inline Vector2u operator+(const Vector2u& v1, const Vector2u& v2)
	{
		Vector2u new_vec = v1;
		new_vec += v2;
		return new_vec;
	}

	inline Vector2u operator-(const Vector2u& v1, const Vector2u& v2)
	{
		Vector2u new_vec = v1;
		new_vec -= v2;
		return new_vec;
	}
} // namespace bowtie