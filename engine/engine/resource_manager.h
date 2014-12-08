#pragma once

#include <cassert>
#include <stdint.h>

#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>

#include "render_resource_handle.h"
#include "resource.h"
#include "resource_type.h"

namespace bowtie
{
struct Material;
struct RenderInterface;
struct Texture;
struct Image;
struct Shader;
class Sprite;
struct Font;

struct ResourceManager
{
	Allocator* allocator;
	RenderInterface* render_interface;
	Hash<Resource> _resources;
	Resource _default_resources[(unsigned)ResourceType::NumResourceTypes];
};

namespace resource_manager
{
	static const char* resource_type_names[] = { "shader", "image", "texture", "font", "material" };
	ResourceType resource_type_from_string(const char* type);

	void init(ResourceManager& rm, Allocator& allocator, RenderInterface& render_interface);
	void deinit(ResourceManager& rm);
	Resource load(ResourceManager& rm, ResourceType type, const char* filename);
	Resource get(const ResourceManager& rm, ResourceType type, uint64_t name);
	void reload(ResourceManager& rm, ResourceType type, const char* filename);
	void reload_all(ResourceManager& rm);
	void set_default(ResourceManager& rm, ResourceType type, Resource resource);
	Resource get_default(const ResourceManager& rm, ResourceType type);
}

} // namespace bowtie