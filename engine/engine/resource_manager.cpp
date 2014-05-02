#include "resource_manager.h"

#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>
#include <foundation/temp_allocator.h>
#include <resource_path.h>

#include "render_interface.h"
#include "bmp.h"
#include "sprite.h"
#include "texture.h"


namespace bowtie
{

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

uint64_t hash_name(const char* name)
{
	return murmur_hash_64(name, strlen32(name), 0); 
}

uint64_t get_shader_name(const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	char concatenated_filenames[1024];
	strcpy(concatenated_filenames, vertex_shader_filename);
	strcat(concatenated_filenames, fragment_shader_filename);

	return murmur_hash_64(concatenated_filenames, strlen32(concatenated_filenames), 0);
}

ResourceHandle ResourceManager::load_shader(const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	char* vertex_shader_source = file::load(vertex_shader_filename, _allocator);
	char* fragment_shader_source = file::load(fragment_shader_filename, _allocator);
	
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
	add_resource(get_shader_name(vertex_shader_filename, fragment_shader_filename), RT_Shader, shader_resource.handle);

	_allocator.deallocate(vertex_shader_source);
	_allocator.deallocate(fragment_shader_source);

	return shader_resource.handle;
}

Image& ResourceManager::load_image(const char* filename)
{
	BmpTexture bmp = bmp::load(filename, _allocator);
	
	Image* image = (Image*)_allocator.allocate(sizeof(Image));
	image->resolution = Vector2u(bmp.width, bmp.height);
	image->data = bmp.data;
	image->data_size = bmp.data_size;
	image->pixel_format = image::RGB;
	
	add_resource(hash_name(filename), RT_Image, image);

	return *image;
}

Texture& ResourceManager::load_texture(const char* filename)
{
	auto& image = load_image(filename);
	auto texture = MAKE_NEW(_allocator, Texture, &image);
	_render_interface.create_texture(*texture);
	add_resource(hash_name(filename), RT_Texture, texture);
	return *texture;	
}

Sprite& ResourceManager::load_sprite_prototype(const char* filename)
{
	auto sprite = MAKE_NEW(_allocator, Sprite, load_texture(filename));
	add_resource(hash_name(filename), RT_Sprite, sprite);
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

}
