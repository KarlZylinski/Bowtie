#pragma once

#include <foundation/vector2.h>
#include <foundation/matrix4.h>

namespace bowtie
{

class View
{
public:
	View(const Vector2& size, const Vector2& position);
	View();
	
	const Vector2& size() const;
	void set_size(const Vector2& size);

	const Vector2& position() const;
	void set_position(const Vector2& position);
	void move(const Vector2& distance);

	Matrix4 projection() const;
	Matrix4 view() const;
	Matrix4 view_projection() const;

private:
	void calculate_projection();

	Vector2 _size;
	Vector2 _position;
};

}