#pragma once

#include "resource_handle.h"

namespace bowtie
{

class Allocator;
struct Vector4;
class IDrawableGeometry
{
public:
	virtual ~IDrawableGeometry() {}
	virtual const IDrawableGeometry& clone(Allocator& allocator) const = 0;
	virtual IDrawableGeometry& clone(Allocator& allocator) = 0;
	virtual const Vector4& color() const = 0;
	virtual const float* data() const = 0;
	virtual unsigned data_size() const = 0;
	virtual bool has_changed() const = 0;
	virtual void reset_has_changed() = 0;
	virtual void set_color(const Vector4& color) = 0;
	virtual const Texture* texture() const = 0;
};

}
