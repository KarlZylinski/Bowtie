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

uniform::Type get_uniform_type_from_str(const char* str)
{
	static const char* types_as_str[] = { "float", "vec2", "vec3", "vec4", "mat3", "mat4", "texture1", "texture2", "texture3" };
	
	for (unsigned i = 0; i < uniform::NumUniformTypes; ++i)
	{
		if (!strcmp(str, types_as_str[i]))
			return (uniform::Type)i;
	}

	assert(!"Unknown uniform type");
	return uniform::NumUniformTypes;
}

uniform::AutomaticValue get_automatic_value_from_str(const char* str)
{
	static const char* types_as_str[] = { "none", "mvp", "mv", "m", "time", "drawable_texture" };
	
	for (unsigned i = 0; i < uniform::NumAutomaticValues; ++i)
	{
		if (!strcmp(str, types_as_str[i]))
			return (uniform::AutomaticValue)i;
	}

	return uniform::None;
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
	auto& uniforms_json = d["uniforms"];

	Array<UniformResourceData> uniforms(_allocator);

	for(auto uniform_iter = uniforms_json.Begin(); uniform_iter != uniforms_json.End(); ++uniform_iter)
	{
		auto& uniform_json = *uniform_iter;

		if (!uniform_json.IsString())
			continue;
		
		TempAllocator4096 ta;
		auto uniform_str = uniform_json.GetString();
		auto split_uniform = split(ta, uniform_str, ' ');
		assert(array::size(split_uniform) >= 2 && "Uniform definition must contain at least type and name.");
		auto type = get_uniform_type_from_str(split_uniform[0]);

		UniformResourceData uniform;
		uniform.type = type;
		uniform.name = copy_str(_allocator, split_uniform[1]);
		uniform.value = nullptr;

		if (array::size(split_uniform) > 2)
		{
			auto value_str = split_uniform[2];
			auto automatic_value = get_automatic_value_from_str(value_str);

			if (automatic_value != uniform::None)
				uniform.automatic_value = automatic_value;
			else
			{
				switch (type)
				{
				case uniform::Float:
					uniform.value = _allocator.construct<float>(float_from_str(value_str));
					break;
				}
			}
		}

		array::push_back(uniforms, uniform);
	}


	/*

	char* current_uniform = (char*)dynamic_data;
	
	for (unsigned i = 0; i < data.num_uniforms; ++i)
	{
		TempAllocator4096 ta;
		auto split_uniform = split(ta, current_uniform, ' ');
		assert(array::size(split_uniform) >= 2 && "Uniform definition must contain at least type and name.");
		auto type = get_uniform_type_from_str(split_uniform[0]);
		auto name = split_uniform[1];
		auto name_hash = hash_str(name);
		auto location = concrete_renderer.get_uniform_location(shader, name);
		
		if (array::size(split_uniform) > 2)
		{
			auto value_str = split_uniform[2];
			auto automatic_value = get_automatic_value_from_str(value_str);

			if (automatic_value != Uniform::None)
				material->add_uniform(Uniform(type, name_hash, location, automatic_value));
			else
			{
				Uniform u(type, name_hash, location);

				switch (type)
				{
				case Uniform::Float:
					uniform::SetValue(u, allocator, get_float_from_str(value_str));
					break;
				}
				
				material->add_uniform(u);
			}
		}
		else
			material->add_uniform(Uniform(type, name_hash, location));

		current_uniform += strlen(current_uniform) + 1;
	}



	*/

	auto num_uniforms = array::size(uniforms);
	auto uniform_data_size = unsigned(num_uniforms * sizeof(UniformResourceData));
	auto uniforms_data = _allocator.allocate(uniform_data_size);
	memcpy(uniforms_data, &uniforms[0], uniform_data_size);
	
	MaterialResourceData mrd;
	mrd.num_uniforms = num_uniforms;
	mrd.shader = shader.render_handle;
	RenderResourceData material_resource_data = _render_interface.create_render_resource_data(RenderResourceData::RenderMaterial);
	material_resource_data.data = &mrd;	
	_render_interface.create_resource(material_resource_data, uniforms_data, uniform_data_size);

	auto material = _allocator.construct<Material>(material_resource_data.handle, &shader);
	add_resource(name, Resource(material));	

	return *material;
}

RenderResourceData get_create_render_resource_data(RenderInterface& render_interface, RenderResourceData::Type type, void* data)
{
	RenderResourceData resource_data = render_interface.create_render_resource_data(type);
	resource_data.data = data;
	return resource_data;
}

RenderResourceData get_update_render_resource_data(RenderResourceData::Type type, RenderResourceHandle handle, void* data)
{
	RenderResourceData resource_data = { type, handle, data };
	return resource_data;
}

template<typename T> struct RenderResourcePackage
{
	RenderResourcePackage(const T& data, void* dynamic_data, unsigned dynamic_data_size)
		: data(data), dynamic_data(dynamic_data), dynamic_data_size(dynamic_data_size)
	{}

	T data;
	void* dynamic_data;
	unsigned dynamic_data_size;
};

RenderResourcePackage<ShaderResourceData> get_shader_resource_data(Allocator& allocator, const char* filename)
{
	auto shader_source = file::load(filename, allocator);
	auto split_shader = shader_utils::split_shader(shader_source, allocator);
	allocator.deallocate(shader_source.data);
	
	ShaderResourceData srd;
	unsigned shader_dynamic_data_size = split_shader.vertex_source_len + split_shader.fragment_source_len;
	unsigned shader_dynamic_data_offset = 0;
	void* shader_resource_dynamic_data = allocator.allocate(shader_dynamic_data_size);

	srd.vertex_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)shader_resource_dynamic_data, (char*)split_shader.vertex_source);
	shader_dynamic_data_offset += split_shader.vertex_source_len;

	srd.fragment_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), (char*)split_shader.fragment_source);
	
	allocator.deallocate(split_shader.vertex_source);
	allocator.deallocate(split_shader.fragment_source);
	return RenderResourcePackage<ShaderResourceData>(srd, shader_resource_dynamic_data, shader_dynamic_data_size);
}

Shader& ResourceManager::load_shader(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(resource_type::Shader, name);

	if (existing.object != nullptr)
		return *(Shader*)existing.object;

	auto resource_package = get_shader_resource_data(_allocator, filename);
	auto create_resource_data = get_create_render_resource_data(_render_interface, RenderResourceData::Shader, &resource_package.data);
	_render_interface.create_resource(create_resource_data, resource_package.dynamic_data, resource_package.dynamic_data_size);
	auto shader = _allocator.construct<Shader>(create_resource_data.handle);
	add_resource(name, Resource(shader));
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
	auto drawable =_allocator.construct<Drawable>(_allocator, *sprite_geometry, &material, 0);
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

void ResourceManager::reload(const char* type, const char* filename)
{
	reload(resource_type_from_string(type), filename);
}

void ResourceManager::reload(ResourceType type, const char* filename)
{
	switch(type)
	{
		case resource_type::Shader:
			{
				auto& shader = *get<Shader>(type, filename);
				auto shader_data = get_shader_resource_data(_allocator, filename);
				auto update_command_data = get_update_render_resource_data(RenderResourceData::Shader, shader.render_handle, &shader_data.data);
				_render_interface.update_resource(update_command_data, shader_data.dynamic_data, shader_data.dynamic_data_size);
			}
			break;
		default:
			assert(!"Tried to reload unsupported resource type");
			break;
	}
}

void ResourceManager::reload_all()
{
	// TODO: Find all resources through FS thingy and send them off to reload.
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
