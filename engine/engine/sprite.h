#pragma once

#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/matrix4.h>

#include "drawable.h"
#include "resource_handle.h"
#include "rect.h"

namespace bowtie
{

class Allocator;
class Sprite : public Drawable
{
public:
	static const unsigned s_geometry_size = sizeof(float) * 9 * 6;

	Sprite(const Texture& texture);

	void set_rect(const Rect& rect);
	const Rect& rect() const;
	
	virtual const Texture* texture() const;
	virtual const float* geometry_data() const;
	virtual unsigned geometry_size() const;

protected:
	virtual void update_geometry();

private:
	float _geometry_data[s_geometry_size];
	const Texture* _texture;
	Rect _rect;
};

}