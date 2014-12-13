#include "view.h"
#include "rect.h"

namespace bowtie
{

namespace view
{

void move(Rect* rect, const Vector2* distance)
{
	rect->position += *distance;
}

Matrix4 projection_matrix(const Rect* rect)
{
	Matrix4 projection_matrix;

	auto near_plane = 0;
	auto far_plane = 1.0f;
	
	projection_matrix[0][0] = 2.0f/(rect->size.x - 1.0f);
	projection_matrix[0][1] = 0;
	projection_matrix[0][2] = 0;
	projection_matrix[0][3] = 0;
	
	projection_matrix[1][0] = 0;
	projection_matrix[1][1] = -2.0f/(rect->size.y - 1.0f);
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

Matrix4 view_matrix(const Rect* rect)
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
	
	view_matrix[3][0] = rect->position.x;
	view_matrix[3][1] = rect->position.y;
	view_matrix[3][2] = 0;
	view_matrix[3][3] = 1;
	
	return view_matrix;
}

Matrix4 view_projection_matrix(const Rect* rect)
{
	return view_matrix(rect) * projection_matrix(rect);
}

} // namespace view;

} // namespace bowtie
