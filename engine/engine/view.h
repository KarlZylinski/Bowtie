#pragma once

#include <foundation/matrix4.h>

namespace bowtie
{

struct Rect;
struct Vector2;

namespace view
{
	void move(Rect* rect, const Vector2* distance);
	Matrix4 projection_matrix(const Rect* rect);
	Matrix4 view_matrix(const Rect* rect);
	Matrix4 view_projection_matrix(const Rect* rect);
}

}