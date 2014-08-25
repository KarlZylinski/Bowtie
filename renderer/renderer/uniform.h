#pragma once

#include <foundation/vector4.h>
#include <stdint.h>

namespace bowtie
{

struct Uniform
{
	enum Type
	{
		Float, Vec2, Vec3, Vec4, Mat3, Mat4, NumUniformTypes
	};

	enum AutomaticValue
	{
		None, ModelViewProjectionMatrix, NumAutomaticValues
	};

	Uniform(Type type, uint64_t name, unsigned location, AutomaticValue automatic_value);
	Uniform(Type type, uint64_t name, unsigned location, Vector4 value);

	AutomaticValue automatic_value;
	uint64_t name;
	unsigned location;
	Type type;
	Vector4 value;
};

}