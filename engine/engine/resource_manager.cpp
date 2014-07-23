#include "resource_manager.h"

#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>
#include <foundation/temp_allocator.h>
#include <resource_path.h>

#include "font.h"
#include "render_interface.h"
#include "png.h"
#include "sprite.h"
#include "texture.h"
#include "shader_utils.h"

namespace bowtie
{


const char* ResourceManager::resource_type_names[] = { "shader", "image", "sprite", "texture", "font" };

ResourceManager::ResourceManager(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface), _resources(allocator)
{
}

ResourceManager::~ResourceManager()
{
	for(auto resource_iter = hash::begin(_resources); resource_iter != hash::end(_resources); ++resource_iter)
	{
		if(resource_iter->value.type != ResourceHandle::Object)
			continue;
		
		auto obj = resource_iter->value.object;
		switch(resource_iter->value.object_type)
		{
			case resource_type::Image: MAKE_DELETE(_allocator, Image, (Image*)obj); break;
			case resource_type::Sprite: MAKE_DELETE(_allocator, Sprite, (Sprite*)obj); break;
			case resource_type::Texture: MAKE_DELETE(_allocator, Texture, (Texture*)obj); break;
			case resource_type::Font: MAKE_DELETE(_allocator, Font, (Font*)obj); break;
			default: assert(!"Some resource type isn't freed properly."); break;
		}
	}
}

ResourceType ResourceManager::resource_type_from_string(const char* type)
{
	for (unsigned i = 0; i < resource_type::NumResourceTypes; ++i)
	{
		if (strequal(type, resource_type_names[i]))
			return (ResourceType)i;
	}

	assert(!"Unknown resource type string");
	return ResourceType::NumResourceTypes;
}


uint64_t hash_name(const char* name)
{
	return murmur_hash_64(name, strlen32(name), 0); 
}

ResourceHandle ResourceManager::load_shader(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(resource_type::Shader, name);
	if (existing.type != ResourceHandle::NotInitialized)
		return existing;

	auto shader_source = file::load(filename, _allocator);

	auto split_shader = shader_utils::split_shader(shader_source, _allocator);
	
	ShaderResourceData srd;
	unsigned shader_dynamic_data_size = split_shader.vertex_source_len + split_shader.fragment_source_len;
	unsigned shader_dynamic_data_offset = 0;
	void* shader_resource_dynamic_data = _allocator.allocate(shader_dynamic_data_size);

	srd.vertex_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)shader_resource_dynamic_data, (char*)split_shader.vertex_source);
	shader_dynamic_data_offset += split_shader.vertex_source_len;

	srd.fragment_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), (char*)split_shader.fragment_source);

	RenderResourceData shader_resource = _render_interface.create_render_resource_data(RenderResourceData::Shader);
	shader_resource.data = &srd;
	
	_render_interface.create_resource(shader_resource, shader_resource_dynamic_data, shader_dynamic_data_size);
	add_resource(name, resource_type::Shader, shader_resource.handle);

	_allocator.deallocate(split_shader.vertex_source);
	_allocator.deallocate(split_shader.fragment_source);
	_allocator.deallocate(shader_source.data);

	return shader_resource.handle;
}

Image& ResourceManager::load_image(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Image>(resource_type::Image, name);
	if (existing != nullptr)
		return *existing;

	UncompressedTexture tex = png::load(filename, _allocator);
	
	auto image = MAKE_NEW(_allocator, Image);
	image->resolution = Vector2u(tex.width, tex.height);
	image->data = tex.data;
	image->data_size = tex.data_size;
	image->pixel_format = image::RGBA;
	
	add_resource(name, resource_type::Image, ResourceHandle(image));

	return *image;
}

Texture& ResourceManager::load_texture(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Texture>(resource_type::Texture, name);
	if (existing != nullptr)
		return *existing;

	auto& image = load_image(filename);
	auto texture = MAKE_NEW(_allocator, Texture, &image);
	_render_interface.create_texture(*texture);
	add_resource(name, resource_type::Texture, ResourceHandle(texture));
	return *texture;	
}

Font& ResourceManager::load_font(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Font>(resource_type::Font, name);
	if (existing != nullptr)
		return *existing;

	auto font = MAKE_NEW(_allocator, Font, const_cast<const Texture&>(load_texture(filename)), 32, 4);
	add_resource(name, resource_type::Font, ResourceHandle(font));
	return *font;
}

Sprite& ResourceManager::load_sprite_prototype(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Sprite>(resource_type::Sprite, name);
	if (existing != nullptr)
		return *existing;

	auto sprite = MAKE_NEW(_allocator, Sprite, load_texture(filename));
	add_resource(name, resource_type::Sprite, ResourceHandle(sprite));
	return *sprite;
}

uint64_t ResourceManager::get_name(uint64_t name, ResourceType type)
{
	char name_str[500];
	sprintf(name_str, "%u%llu", (unsigned)type, name);
	::uint64_t name_with_type;
	sscanf(name_str, "%llu", &name_with_type);
	return name_with_type;
}

void ResourceManager::add_resource(uint64_t name, ResourceType type, ResourceHandle resource)
{
	hash::set(_resources, get_name(name, type), resource);
}

ResourceHandle ResourceManager::get(ResourceType type, uint64_t name)
{
	return hash::get(_resources, get_name(name, type), ResourceHandle());
}

ResourceHandle ResourceManager::load(ResourceType type, const char* filename)
{
	switch(type)
	{
		case resource_type::Image: return ResourceHandle(&load_image(filename));
		case resource_type::Shader: return load_shader(filename);
		case resource_type::Sprite: return ResourceHandle(&load_sprite_prototype(filename));
		case resource_type::Texture: return ResourceHandle(&load_texture(filename));
		case resource_type::Font: return ResourceHandle(&load_font(filename));
		default: assert(!"Unknown resource type"); return ResourceHandle();
	}
}

ResourceHandle ResourceManager::load(const char* type, const char* filename)
{
	return load(resource_type_from_string(type), filename);
}

void ResourceManager::set_default(ResourceType type, ResourceHandle handle)
{
	assert(_default_resources[type].type == ResourceHandle::NotInitialized && "Trying to resassign already assigned default resource.");
	_default_resources[type] = handle;
}

ResourceHandle ResourceManager::get_default(ResourceType type) const
{
	auto resource = _default_resources[type];
	assert(resource.type != ResourceHandle::NotInitialized && "No default resource set for this type.");
	return resource;
}

}
