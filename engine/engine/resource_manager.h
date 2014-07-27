#pragma once

#include <cassert>
#include <stdint.h>

#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>

#include "resource_handle.h"

namespace bowtie
{
class RenderInterface;
struct Texture;
struct Image;
class Sprite;
class Font;
class ResourceManager
{
public:
	ResourceManager(Allocator& allocator, RenderInterface& render_interface);
	~ResourceManager();
		
	static const char* resource_type_names[];
	static ResourceType resource_type_from_string(const char* type);
	
	ResourceHandle load(const char* type, const char* filename);
	ResourceHandle load(ResourceType type, const char* filename);
	ResourceHandle get(ResourceType type, uint64_t name);
	ResourceHandle get(const char* type, uint64_t name);
	ResourceHandle get(const char* type, const char* name);
	ResourceHandle get(ResourceType type, const char* name);

	template<class T> T* get(ResourceType type, uint64_t name)
	{
		auto name_with_type = get_name(name, type);

		if (!hash::has(_resources, name_with_type))
			return nullptr;

		ResourceHandle handle = hash::get(_resources, name_with_type);
		assert(handle.type == ResourceHandle::Object && "Trying to get resource as object, which it isn't");
		return (T*)handle.object;
	}
	
	template<class T> T* get(ResourceType type, const char* name)
	{
		return get<T>(type, murmur_hash_64(name, strlen32(name), 0));
	}

	void set_default(ResourceType type, ResourceHandle handle);
	ResourceHandle get_default(ResourceType type) const;
	
private:
	uint64_t get_name(uint64_t name, ResourceType type);
	void add_resource(uint64_t name, ResourceType type, ResourceHandle resource);

	ResourceHandle load_shader(const char* filename);
	Image& load_image(const char* filename);
	Drawable& load_sprite_prototype(const char* filename);
	Texture& load_texture(const char* filename);
	Font& load_font(const char* filename);

	Allocator& _allocator;
	RenderInterface& _render_interface;

	Hash<ResourceHandle> _resources;
	ResourceHandle _default_resources[resource_type::NumResourceTypes];

	ResourceManager(const ResourceManager&);
	ResourceManager& operator=(const ResourceManager&);
};


} // namespace bowtie