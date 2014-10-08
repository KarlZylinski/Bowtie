#include "render_uniform.h"

namespace bowtie
{

RenderUniform::RenderUniform(uniform::Type type, uint64_t name, unsigned location, uniform::AutomaticValue automatic_value) :
	automatic_value(automatic_value), name(name), location(location), type(type), value(nullptr)
{}


RenderUniform::RenderUniform(uniform::Type type, uint64_t name, unsigned location) :
	name(name), location(location), type(type), value(nullptr)
{}

}