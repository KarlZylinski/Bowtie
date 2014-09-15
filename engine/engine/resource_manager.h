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
class Drawable;
struct Material;
class RenderInterface;
struct Texture;
struct Image;
struct Shader;
class Sprite;
class Font;
class ResourceManager
{
public:
	ResourceManager(Allocator& allocator, RenderInterface& render_interface);
	~ResourceManager();
		
	static const char* resource_type_names[];
	static ResourceType resource_type_from_string(const char* type);
	
	Resource load(const char* type, const char* filename);
	Resource load(ResourceType type, const char* filename);
	Resource get(ResourceType type, uint64_t name);
	Resource get(const char* type, uint64_t name);
	Resource get(const char* type, const char* name);
	Resource get(ResourceType type, const char* name);

	template<class T> T* get(ResourceType type, uint64_t name)
	{
		auto name_with_type = get_name(name, type);

		if (!hash::has(_resources, name_with_type))
			return nullptr;
		
		Resource resource = hash::get(_resources, name_with_type);
		assert(resource.type != resource_type::NotInitialized && resource.object != 0 && "Tried to get uninitialized resource");
		return (T*)resource.object;
	}
	
	template<class T> T* get(ResourceType type, const char* name)
	{
		return get<T>(type, hash_str(name));
	}

	void reload(const char* type, const char* filename);
	void reload(ResourceType type, const char* filename);
	void set_default(ResourceType type, Resource resource);
	Resource get_default(ResourceType type) const;
	
private:
	uint64_t get_name(uint64_t name, ResourceType type);
	void add_resource(uint64_t name, Resource resource);

	Material& load_material(const char* filename);
	Shader& load_shader(const char* filename);
	Image& load_image(const char* filename);
	Drawable& load_sprite_prototype(const char* filename);
	Texture& load_texture(const char* filename);
	Font& load_font(const char* filename);

	Allocator& _allocator;
	RenderInterface& _render_interface;

	Hash<Resource> _resources;
	Resource _default_resources[resource_type::NumResourceTypes];

	ResourceManager(const ResourceManager&);
	ResourceManager& operator=(const ResourceManager&);
};


} // namespace bowtie