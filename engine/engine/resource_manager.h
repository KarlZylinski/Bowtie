#pragma once

#include <cassert>
#include <stdint.h>

#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>

#include "render_resource_handle.h"
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
	Hash<void*> _resources;
	Option<void*> _default_resources[(unsigned)ResourceType::NumResourceTypes];
};

namespace resource_manager
{
	static const char* resource_type_names[] = { "not_initialized", "shader", "image", "texture", "font", "material" };
	ResourceType resource_type_from_string(const char* type);

	void init(ResourceManager& rm, Allocator& allocator, RenderInterface& render_interface);
	void deinit(ResourceManager& rm);
	Option<void*> load(ResourceManager& rm, ResourceType type, const char* filename);
	Option<void*> get(const ResourceManager& rm, ResourceType type, uint64_t name);
	void reload(ResourceManager& rm, ResourceType type, const char* filename);
	void reload_all(ResourceManager& rm);
	void set_default(ResourceManager& rm, ResourceType type, void* resource);
	Option<void*> get_default(const ResourceManager& rm, ResourceType type);
}

} // namespace bowtie