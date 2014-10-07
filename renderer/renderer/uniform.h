#pragma once

#include <foundation/vector4.h>
#include <stdint.h>

namespace bowtie
{

struct Uniform
{
	enum Type
	{
		Float, Vec2, Vec3, Vec4, Mat3, Mat4, Texture1, Texture2, Texture3, NumUniformTypes
	};

	enum AutomaticValue
	{
		None, ModelViewProjectionMatrix, ModelViewMatrix, ModelMatrix, Time, DrawableTexture, NumAutomaticValues
	};

	Uniform(Type type, uint64_t name, unsigned location, AutomaticValue automatic_value);
	Uniform(Type type, uint64_t name, unsigned location);

	AutomaticValue automatic_value;
	uint64_t name;
	unsigned location;
	Type type;
	void* value;
};

}