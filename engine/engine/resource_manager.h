#pragma once

#include <stdint.h>
#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>
#include "render_resource_types.h"

namespace bowtie
{
class RenderInterface;
struct Texture;
struct Image;
class Sprite;
class ResourceManager
{
public:
	enum ResourceType
	{
		RT_Shader, RT_Image, RT_Sprite, RT_Texture
	};

	ResourceManager(Allocator& allocator, RenderInterface& render_interface);
	~ResourceManager();

	ResourceHandle load_shader(const char* vertex_shader_filename, const char* fragment_shader_filename);
	Image& load_image(const char* filename);
	Sprite& load_sprite_prototype(const char* filename);
	Texture& load_texture(const char* filename);

	ResourceHandle get(ResourceType type, uint64_t name);

	template<class T> T* get(ResourceType type, uint64_t name)
	{
		ResourceHandle handle = hash::get(_resources, get_name(name, type), ResourceHandle());
		assert(handle.type == ResourceHandle::Object && "Trying to get resource as object, which it isn't");
		return (T*)handle.object;
	}
	
	template<class T> T* get(ResourceType type, const char* name)
	{
		return get<T>(type, murmur_hash_64(name, strlen32(name), 0));
	}
	
private:
	uint64_t get_name(uint64_t name, ResourceType type);

	ResourceManager(const ResourceManager&);
	ResourceManager& operator=(const ResourceManager&);
	
	void add_resource(uint64_t name, ResourceType type, ResourceHandle resource);

	Allocator& _allocator;
	RenderInterface& _render_interface;

	Hash<ResourceHandle> _resources;
};


} // namespace bowtie