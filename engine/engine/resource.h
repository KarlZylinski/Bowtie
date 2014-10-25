#pragma once

#include "resource_type.h"

namespace bowtie
{

class Drawable;
struct Font;
struct Image;
struct Material;
struct Shader;
struct Texture;

struct Resource
{
	Resource() : object(0), type(ResourceType::NotInitialized) {}
	explicit Resource(Shader* t) : object(t), type(ResourceType::Shader) {}
	explicit Resource(Material* i) : object(i), type(ResourceType::Material) {}
	explicit Resource(Texture* t) : object(t), type(ResourceType::Texture) {}
	explicit Resource(Image* i) : object(i), type(ResourceType::Image) {}
	explicit Resource(Drawable* d) : object(d), type(ResourceType::Drawable) {}
	explicit Resource(Font* f) : object(f),  type(ResourceType::Font) {}

	ResourceType type;
	void* object;
};

} // namespace bowtie