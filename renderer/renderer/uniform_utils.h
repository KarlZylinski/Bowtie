#pragma once

namespace bowtie
{

struct Vector4;
class Allocator;
struct Uniform;

namespace uniform
{
	void SetValue(Uniform& uniform, Allocator& allocator, const Vector4& value);
	void SetValue(Uniform& uniform, Allocator& allocator, unsigned value);
	void SetValue(Uniform& uniform, Allocator& allocator, float value);
};

}