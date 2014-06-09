#pragma once

namespace bowtie
{
	struct Vector4
	{
		Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Vector4() : x(0), y(0), z(0), w(0) {}
		
		inline void operator+=(const Vector4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
		}
						
		union { float x, r; };
		union { float y, g; };
		union { float z, b; };
		union { float w, a; };
	};

	typedef Vector4 Color;

	inline Vector4 operator-(const Vector4& v)
	{
		return Vector4(-v.x, -v.y, -v.z, -v.w);
	}

	inline Vector4 operator+(const Vector4& v1, const Vector4& v2)
	{
		Vector4 new_vec = v1;
		new_vec += v2;
		return new_vec;
	}

	inline float dot(const Vector4& v1, const Vector4& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}

} // namespace bowtie