#pragma once

#include "resource_type.h"

namespace bowtie
{

class Drawable;
class Font;
struct Image;
struct Material;
struct Shader;
struct Texture;

struct Resource
{
	Resource() : object(0), type(resource_type::NotInitialized) {}
	explicit Resource(Shader* t) : object(t), type(resource_type::Shader) {}
	explicit Resource(Material* i) : object(i), type(resource_type::Material) {}
	explicit Resource(Texture* t) : object(t), type(resource_type::Texture) {}
	explicit Resource(Image* i) : object(i), type(resource_type::Image) {}
	explicit Resource(Drawable* d) : object(d), type(resource_type::Drawable) {}
	explicit Resource(Font* f) : object(f),  type(resource_type::Font) {}

	ResourceType type;
	void* object;
};

} // namespace bowtie