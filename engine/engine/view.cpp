#include "view.h"

namespace bowtie
{

View::View(const Vector2& size, const Vector2& position) : _size(size), _position(-position)
{
	
}

View::View()
{
	
}
	
const Vector2& View::size() const
{
	return _size;
}

void View::set_size(const Vector2& size)
{
	_size = size;
}

const Vector2& View::position() const
{
	return _position;
}

void View::set_position(const Vector2& position)
{
	_position = -position;
}

void View::set_position(float x, float y)
{
	_position.x = -x;
	_position.y = -y;
}

void View::move(const Vector2& distance)
{
	_position -= distance;
}

void View::move(float x, float y)
{
	_position.x -= x;
	_position.y -= y;
}

Matrix4 View::projection() const
{	
	Matrix4 projection_matrix;

	auto near_plane = 0.0f;
	auto far_plane = 1.0f;
	
	projection_matrix[0][0] = 2.0f/(_size.x - 1.0f);
	projection_matrix[0][1] = 0;
	projection_matrix[0][2] = 0;
	projection_matrix[0][3] = 0;
	
	projection_matrix[1][0] = 0;
	projection_matrix[1][1] = -2.0f/(_size.y - 1.0f);
	projection_matrix[1][2] = 0;
	projection_matrix[1][3] = 0;

	projection_matrix[2][0] = 0;
	projection_matrix[2][1] = 0;
	projection_matrix[2][2] = 2.0f/(far_plane/near_plane);
	projection_matrix[2][3] = 0;
	
	projection_matrix[3][0] = -1;
	projection_matrix[3][1] = 1;
	projection_matrix[3][2] = (near_plane+far_plane)/(near_plane-far_plane);
	projection_matrix[3][3] = 1;
	
	return projection_matrix;
}

Matrix4 View::view() const
{
	Matrix4 view_matrix;
		
	view_matrix[0][0] = 1;
	view_matrix[0][1] = 0;
	view_matrix[0][2] = 0;
	view_matrix[0][3] = 0;
	
	view_matrix[1][0] = 0;
	view_matrix[1][1] = 1;
	view_matrix[1][2] = 0;
	view_matrix[1][3] = 0;

	view_matrix[2][0] = 0;
	view_matrix[2][1] = 0;
	view_matrix[2][2] = 1;
	view_matrix[2][3] = 0;
	
	view_matrix[3][0] = _position.x;
	view_matrix[3][1] = _position.y;
	view_matrix[3][2] = 0;
	view_matrix[3][3] = 1;
	
	return view_matrix;
}

Matrix4 View::view_projection() const
{
	return view() * projection();
}

} // namespace bowtie