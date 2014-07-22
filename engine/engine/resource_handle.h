#pragma once

#include "resource_type.h"

namespace bowtie
{

struct Texture;
struct Image;
class Sprite;
class Font;

struct ResourceHandle
{
	ResourceHandle() : type(NotInitialized) {}
	explicit ResourceHandle(unsigned h) : handle(h), type(Handle), object_type(resource_type::NumResourceTypes) {}
	explicit ResourceHandle(Texture* t) : object(t), type(Object), object_type(resource_type::Texture) {}
	explicit ResourceHandle(Image* i) : object(i), type(Object), object_type(resource_type::Image) {}
	explicit ResourceHandle(Sprite* s) : object(s), type(Object), object_type(resource_type::Sprite) {}
	explicit ResourceHandle(Font* f) : object(f), type(Object), object_type(resource_type::Font) {}

	enum ResourceHandleType
	{
		NotInitialized, Handle, Object
	};

	ResourceHandleType type;
	ResourceType object_type;
	union
	{
		void* object;
		unsigned handle;
	};
};

} // namespace bowtie