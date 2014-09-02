#include "resource_manager.h"

#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/murmur_hash.h>
#include <foundation/rapidjson/document.h>
#include <foundation/string_utils.h>
#include <foundation/temp_allocator.h>
#include <resource_path.h>

#include "drawable.h"
#include "font.h"
#include "material.h"
#include "png.h"
#include "render_interface.h"
#include "shader.h"
#include "shader_utils.h"
#include "sprite_geometry.h"
#include "texture.h"

namespace bowtie
{


const char* ResourceManager::resource_type_names[] = { "shader", "image", "sprite", "texture", "font", "drawable", "material" };

ResourceManager::ResourceManager(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface), _resources(allocator)
{
}

ResourceManager::~ResourceManager()
{
	for(auto resource_iter = hash::begin(_resources); resource_iter != hash::end(_resources); ++resource_iter)
	{
		auto obj = resource_iter->value.object;
		switch(resource_iter->value.type)
		{
			case resource_type::Material: _allocator.destroy((Material*)obj); break;
			case resource_type::Shader: _allocator.destroy((Shader*)obj); break;
			case resource_type::Image: _allocator.destroy((Image*)obj); break;
			case resource_type::Drawable: _allocator.destroy((Drawable*)obj); break;
			case resource_type::Texture: _allocator.destroy((Texture*)obj); break;
			case resource_type::Font: _allocator.destroy((Font*)obj); break;
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
	return hash_str(name); 
}

Material& ResourceManager::load_material(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(resource_type::Material, name);

	if (existing.object != nullptr)
		return *(Material*)existing.object;

	LoadedFile file = file::load(filename, _allocator);
	using namespace rapidjson;
    Document d;	
    d.Parse<0>((char*)file.data);
	_allocator.deallocate(file.data);

	auto shader_filename = d["shader"].GetString();
	auto& shader = load_shader(shader_filename);
	auto& uniforms = d["uniforms"];
	unsigned uniforms_dynamic_data_size = 0;
	unsigned num_uniforms = 0;

	for(auto uniform_iter = uniforms.Begin(); uniform_iter != uniforms.End(); ++uniform_iter)
	{
		auto& uniform = *uniform_iter;

		if (!uniform.IsString())
			continue;

		uniforms_dynamic_data_size += uniform.GetStringLength() + 1;
		++num_uniforms;
	}

	char* uniforms_dynamic_data = num_uniforms != 0
		? (char*)_allocator.allocate(uniforms_dynamic_data_size)
		: nullptr;

	char* uniforms_dynamic_data_writer = uniforms_dynamic_data;
	for(auto uniform_iter = uniforms.Begin(); uniform_iter != uniforms.End(); ++uniform_iter)
	{
		auto& uniform = *uniform_iter;

		if (!uniform.IsString())
			continue;
		
		auto uniform_str = uniform.GetString();
		unsigned uniform_str_len = uniform.GetStringLength() + 1;
		memcpy(uniforms_dynamic_data_writer, uniform_str, uniform_str_len);
		uniforms_dynamic_data_writer += uniform_str_len;
	}
	
	MaterialResourceData mrd;
	mrd.num_uniforms = num_uniforms;
	mrd.shader = shader.render_handle;
	RenderResourceData material_resource_data = _render_interface.create_render_resource_data(RenderResourceData::RenderMaterial);
	material_resource_data.data = &mrd;	
	_render_interface.create_resource(material_resource_data, uniforms_dynamic_data, uniforms_dynamic_data_size);

	auto material = _allocator.construct<Material>(material_resource_data.handle, &shader);
	add_resource(name, Resource(material));	

	return *material;
}

Shader& ResourceManager::load_shader(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(resource_type::Shader, name);

	if (existing.object != nullptr)
		return *(Shader*)existing.object;

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

	RenderResourceData shader_resource_data = _render_interface.create_render_resource_data(RenderResourceData::Shader);
	shader_resource_data.data = &srd;
	_render_interface.create_resource(shader_resource_data, shader_resource_dynamic_data, shader_dynamic_data_size);

	auto shader = _allocator.construct<Shader>(shader_resource_data.handle);
	add_resource(name, Resource(shader));

	_allocator.deallocate(split_shader.vertex_source);
	_allocator.deallocate(split_shader.fragment_source);
	_allocator.deallocate(shader_source.data);

	return *shader;
}

Image& ResourceManager::load_image(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Image>(resource_type::Image, name);
	if (existing != nullptr)
		return *existing;

	UncompressedTexture tex = png::load(filename, _allocator);
	
	auto image = _allocator.construct<Image>();
	image->resolution = Vector2u(tex.width, tex.height);
	image->data = tex.data;
	image->data_size = tex.data_size;
	image->pixel_format = image::RGBA;
	
	add_resource(name, Resource(image));

	return *image;
}

Texture& ResourceManager::load_texture(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Texture>(resource_type::Texture, name);
	if (existing != nullptr)
		return *existing;

	auto& image = load_image(filename);
	auto texture = _allocator.construct<Texture>(&image);
	_render_interface.create_texture(*texture);
	add_resource(name, Resource(texture));
	return *texture;	
}

Font& ResourceManager::load_font(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Font>(resource_type::Font, name);

	if (existing != nullptr)
		return *existing;
	
	LoadedFile file = file::load(filename, _allocator);
	using namespace rapidjson;
    Document d;	
    d.Parse<0>((char*)file.data);
	_allocator.deallocate(file.data);
	
	auto texture_filename = d["texture"].GetString();
	auto columns = d["columns"].GetInt();
	auto rows = d["rows"].GetInt();

	auto font = _allocator.construct<Font>(columns, rows, const_cast<const Texture&>(load_texture(texture_filename)));
	add_resource(name, Resource(font));
	return *font;
}

Drawable& ResourceManager::load_sprite_prototype(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Drawable>(resource_type::Drawable, name);
	if (existing != nullptr)
		return *existing;

	LoadedFile file = file::load(filename, _allocator);
	using namespace rapidjson;
    Document d;	
    d.Parse<0>((char*)file.data);
	_allocator.deallocate(file.data);
	
	auto texture_filename = d["texture"].GetString();
	auto material_filename = d["material"].GetString();

	auto sprite_geometry = _allocator.construct<SpriteGeometry>(load_texture(texture_filename));
	auto& material = load_material(material_filename);
	auto drawable =_allocator.construct<Drawable>(_allocator, *sprite_geometry, &material);
	add_resource(name, Resource(drawable));
	return *drawable;
}

uint64_t ResourceManager::get_name(uint64_t name, ResourceType type)
{
	char name_str[30];
	sprintf(name_str, "%u%llu", (unsigned)type, name);
	::uint64_t name_with_type;
	sscanf(name_str, "%llu", &name_with_type);
	return name_with_type;
}

void ResourceManager::add_resource(uint64_t name, Resource resource)
{
	hash::set(_resources, get_name(name, resource.type), resource);
}

Resource ResourceManager::get(ResourceType type, uint64_t name)
{
	return hash::get(_resources, get_name(name, type), Resource());
}

Resource ResourceManager::load(ResourceType type, const char* filename)
{
	switch(type)
	{
		case resource_type::Material: return Resource(&load_material(filename));
		case resource_type::Image: return Resource(&load_image(filename));
		case resource_type::Shader: return Resource(&load_shader(filename));
		case resource_type::Sprite: return Resource(&load_sprite_prototype(filename));
		case resource_type::Texture: return Resource(&load_texture(filename));
		case resource_type::Font: return Resource(&load_font(filename));
		default: assert(!"Unknown resource type"); return Resource();
	}
}

Resource ResourceManager::load(const char* type, const char* filename)
{
	return load(resource_type_from_string(type), filename);
}

void ResourceManager::set_default(ResourceType type, Resource resource)
{
	assert(_default_resources[type].object == 0 && "Trying to resassign already assigned default resource.");
	_default_resources[type] = resource;
}

Resource ResourceManager::get_default(ResourceType type) const
{
	auto resource = _default_resources[type];
	assert(resource.object != 0 && "No default resource set for this type.");
	return resource;
}

}
