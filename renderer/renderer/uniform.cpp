#include "uniform.h"

namespace bowtie
{

Uniform::Uniform(Type type, uint64_t name, unsigned location, AutomaticValue automatic_value) :
	automatic_value(automatic_value), name(name), location(location), type(type)
{}


Uniform::Uniform(Type type, uint64_t name, unsigned location, Vector4 value) :
	name(name), location(location), type(type), value(value)
{}

}