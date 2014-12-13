#include "matrix4.h"

namespace bowtie
{
namespace matrix4
{
	
Matrix4 indentity()
{
	Matrix4 m = {};
	m.x.x = 1;
	m.y.y = 1;
	m.z.z = 1;
	m.w.w = 1;
	return m;
}

Matrix4 mul(const Matrix4* m1, const Matrix4* m2)
{
	Matrix4 m2m1;
		
	m2m1.x.x = m1->x.x * m2->x.x + m1->x.y * m2->y.x + m1->x.z * m2->z.x + m1->x.w * m2->w.x;
	m2m1.x.y = m1->x.x * m2->x.y + m1->x.y * m2->y.y + m1->x.z * m2->z.y + m1->x.w * m2->w.y;
	m2m1.x.z = m1->x.x * m2->x.z + m1->x.y * m2->y.z + m1->x.z * m2->z.z + m1->x.w * m2->w.z;
	m2m1.x.w = m1->x.x * m2->x.w + m1->x.y * m2->y.w + m1->x.z * m2->z.w + m1->x.w * m2->w.w;
	
	m2m1.y.x = m1->y.x * m2->x.x + m1->y.y * m2->y.x + m1->y.z * m2->z.x + m1->y.w * m2->w.x;
	m2m1.y.y = m1->y.x * m2->x.y + m1->y.y * m2->y.y + m1->y.z * m2->z.y + m1->y.w * m2->w.y;
	m2m1.y.z = m1->y.x * m2->x.z + m1->y.y * m2->y.z + m1->y.z * m2->z.z + m1->y.w * m2->w.z;
	m2m1.y.w = m1->y.x * m2->x.w + m1->y.y * m2->y.w + m1->y.z * m2->z.w + m1->y.w * m2->w.w;
	
	m2m1.z.x = m1->z.x * m2->x.x + m1->z.y * m2->y.x + m1->z.z * m2->z.x + m1->z.w * m2->w.x;
	m2m1.z.y = m1->z.x * m2->x.y + m1->z.y * m2->y.y + m1->z.z * m2->z.y + m1->z.w * m2->w.y;
	m2m1.z.z = m1->z.x * m2->x.z + m1->z.y * m2->y.z + m1->z.z * m2->z.z + m1->z.w * m2->w.z;
	m2m1.z.w = m1->z.x * m2->x.w + m1->z.y * m2->y.w + m1->z.z * m2->z.w + m1->z.w * m2->w.w;
	
	m2m1.w.x = m1->w.x * m2->x.x + m1->w.y * m2->y.x + m1->w.z * m2->z.x + m1->w.w * m2->w.x;
	m2m1.w.y = m1->w.x * m2->x.y + m1->w.y * m2->y.y + m1->w.z * m2->z.y + m1->w.w * m2->w.y;
	m2m1.w.z = m1->w.x * m2->x.z + m1->w.y * m2->y.z + m1->w.z * m2->z.z + m1->w.w * m2->w.z;
	m2m1.w.w = m1->w.x * m2->x.w + m1->w.y * m2->y.w + m1->w.z * m2->z.w + m1->w.w * m2->w.w;

	return m2m1;
}

Vector4 mul(const Matrix4* m, const Vector4* v)
{
	Vector4 mv;

	mv.x = m->x.x * v->x + m->y.x * v->y + m->z.x * v->z + m->w.x * v->w;
	mv.y = m->x.y * v->x + m->y.y * v->y + m->z.y * v->z + m->w.y * v->w;
	mv.z = m->x.z * v->x + m->y.z * v->y + m->z.z * v->z + m->w.z * v->w;
	mv.w = m->x.w * v->x + m->y.w * v->y + m->z.w * v->z + m->w.w * v->w;

	return mv;
}

} // namespace matrix4
} // namespace bowtie
