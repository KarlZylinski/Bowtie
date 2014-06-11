#pragma once

#include <foundation/vector2.h>
#include <foundation/matrix4.h>
#include "rect.h"

namespace bowtie
{

class View
{
public:
	View(const Rect& rect);
	View();
	
	const Vector2& size() const;
	void set_size(const Vector2& size);

	const Vector2& position() const;
	void set_position(const Vector2& position);
	void set_position(float x, float y);
	void move(const Vector2& distance);
	void move(float x, float y);

	const Rect& rect() const;
	void set_rect(const Rect& rect);

	Matrix4 projection() const;
	Matrix4 view() const;
	Matrix4 view_projection() const;

private:
	void calculate_projection();

	Rect _rect;
};

}