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
		if(resource_iter->value.type == ResourceHandle::Object)
			_allocator.deallocate(resource_iter->value.object);
	}
}

ResourceManager::ResourceType ResourceManager::resource_type_from_string(const char* type)
{
	for (unsigned i = 0; i < NumResourceTypes; ++i)
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
	auto existing = get(RT_Shader, name);
	if (existing.type != ResourceHandle::NotInitialized)
		return existing;

	char* shader_source = file::load(filename, _allocator);

	const char* delimiter = "#fragment";

	auto delimiter_len = strlen32(delimiter);
	auto shader_len = strlen32(shader_source);

	unsigned matched_index = 0;
	unsigned fragment_start_index = 0;
	for (; fragment_start_index < shader_len; ++fragment_start_index)
	{
		if (shader_source[fragment_start_index] == delimiter[matched_index])
			++matched_index;
		else
			matched_index = 0;

		if (matched_index == delimiter_len)
			break;
	}

	++fragment_start_index;
	assert(matched_index == delimiter_len && fragment_start_index <= shader_len && "Could not find fragment part of shader.");

	unsigned vertex_shader_source_len = fragment_start_index - delimiter_len;
	char* vertex_shader_source = (char*)_allocator.allocate(vertex_shader_source_len + 1);
	memcpy(vertex_shader_source, shader_source, vertex_shader_source_len);
	vertex_shader_source[vertex_shader_source_len] = 0;
	
	unsigned fragment_shader_source_len = shader_len - fragment_start_index;
	char* fragment_shader_source = (char*)_allocator.allocate(fragment_shader_source_len + 1);
	memcpy(fragment_shader_source, shader_source + fragment_start_index, fragment_shader_source_len);
	fragment_shader_source[fragment_shader_source_len] = 0;
	
	ShaderResourceData srd;
	unsigned shader_dynamic_data_size = strlen32(vertex_shader_source) + strlen32(fragment_shader_source) + 2;
	unsigned shader_dynamic_data_offset = 0;
	void* shader_resource_dynamic_data = _allocator.allocate(shader_dynamic_data_size);

	srd.vertex_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)shader_resource_dynamic_data, vertex_shader_source);
	shader_dynamic_data_offset += strlen32(vertex_shader_source) + 1;

	srd.fragment_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), fragment_shader_source);

	RenderResourceData shader_resource = _render_interface.create_render_resource_data(RenderResourceData::Shader);
	shader_resource.data = &srd;
	
	_render_interface.create_resource(shader_resource, shader_resource_dynamic_data, shader_dynamic_data_size);
	add_resource(name, RT_Shader, shader_resource.handle);

	_allocator.deallocate(vertex_shader_source);
	_allocator.deallocate(fragment_shader_source);

	return shader_resource.handle;
}

Image& ResourceManager::load_image(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Image>(RT_Image, name);
	if (existing != nullptr)
		return *existing;

	UncompressedTexture tex = png::load(filename, _allocator);
	
	Image* image = (Image*)_allocator.allocate(sizeof(Image));
	image->resolution = Vector2u(tex.width, tex.height);
	image->data = tex.data;
	image->data_size = tex.data_size;
	image->pixel_format = image::RGBA;
	
	add_resource(name, RT_Image, image);

	return *image;
}

Texture& ResourceManager::load_texture(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Texture>(RT_Texture, name);
	if (existing != nullptr)
		return *existing;

	auto& image = load_image(filename);
	auto texture = MAKE_NEW(_allocator, Texture, &image);
	_render_interface.create_texture(*texture);
	add_resource(name, RT_Texture, texture);
	return *texture;	
}

Font& ResourceManager::load_font(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Font>(RT_Font, name);
	if (existing != nullptr)
		return *existing;

	auto font = MAKE_NEW(_allocator, Font, const_cast<const Texture&>(load_texture(filename)));
	add_resource(name, RT_Font, font);
	return *font;
}

Sprite& ResourceManager::load_sprite_prototype(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Sprite>(RT_Sprite, name);
	if (existing != nullptr)
		return *existing;

	auto sprite = MAKE_NEW(_allocator, Sprite, load_texture(filename));
	add_resource(name, RT_Sprite, sprite);
	return *sprite;
}

uint64_t ResourceManager::get_name(uint64_t name, ResourceManager::ResourceType type)
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
		case RT_Image: return &load_image(filename);
		case RT_Shader: return load_shader(filename);
		case RT_Sprite: return &load_sprite_prototype(filename);
		case RT_Texture: return &load_texture(filename);
		case RT_Font: return &load_font(filename);
		default: assert(!"Unknown resource type"); return (unsigned)0;
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
