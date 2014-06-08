#pragma once

#include <foundation/vector2.h>

namespace bowtie
{

struct Rect
{
	Rect(const Vector2& position, const Vector2& size) : position(position), size(size)
	{
	}

	Rect() {}

	Vector2 position;
	Vector2 size;
};

}