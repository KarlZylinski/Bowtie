#include "view.h"
#include "rect.h"

namespace bowtie
{

namespace view
{

void move(Rect* rect, const Vector2* distance)
{
	vector2::inc(&rect->position, distance);
}

Matrix4 projection_matrix(const Rect* rect)
{
	Matrix4 projection_matrix;

	auto near_plane = 0;
	auto far_plane = 1.0f;
	
	projection_matrix.x.x = 2.0f/(rect->size.x - 1.0f);
	projection_matrix.x.y = 0;
	projection_matrix.x.z = 0;
	projection_matrix.x.w = 0;
	
	projection_matrix.y.x = 0;
	projection_matrix.y.y = -2.0f/(rect->size.y - 1.0f);
	projection_matrix.y.z = 0;
	projection_matrix.y.w = 0;

	projection_matrix.z.x = 0;
	projection_matrix.z.y = 0;
	projection_matrix.z.z = 2.0f/(far_plane/near_plane);
	projection_matrix.z.w = 0;
	
	projection_matrix.w.x = -1;
	projection_matrix.w.y = 1;
	projection_matrix.w.z = (near_plane+far_plane)/(near_plane-far_plane);
	projection_matrix.w.w = 1;
	
	return projection_matrix;
}

Matrix4 view_matrix(const Rect* rect)
{
	Matrix4 view_matrix;

	view_matrix.x.x = 1;
	view_matrix.x.y = 0;
	view_matrix.x.z = 0;
	view_matrix.x.w = 0;
	
	view_matrix.y.x = 0;
	view_matrix.y.y = 1;
	view_matrix.y.z = 0;
	view_matrix.y.w = 0;

	view_matrix.z.x = 0;
	view_matrix.z.y = 0;
	view_matrix.z.z = 1;
	view_matrix.z.w = 0;
	
	view_matrix.w.x = rect->position.x;
	view_matrix.w.y = rect->position.y;
	view_matrix.w.z = 0;
	view_matrix.w.w = 1;
	
	return view_matrix;
}

} // namespace view;

} // namespace bowtie
