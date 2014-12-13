#pragma once

#include <foundation/vector2.h>

namespace bowtie
{

struct Rect
{
	Vector2 position;
	Vector2 size;
};

namespace rect
{
	void init(Rect* rect, Vector2* position, Vector2* size);
}

}