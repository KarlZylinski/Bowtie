#include "resource_manager.h"

#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/murmur_hash.h>
#include <foundation/jzon.h>
#include <foundation/string_utils.h>
#include <foundation/temp_allocator.h>
#include <foundation/stream.h>
#include <resource_path.h>

#include "font.h"
#include "material.h"
#include "png.h"
#include "render_interface.h"
#include "shader.h"
#include "shader_utils.h"
#include "texture.h"

static bowtie::Allocator* static_allocator;
static JzonAllocator jzon_allocator;

static void* jzon_static_allocate(size_t size)
{
	return static_allocator->allocate((uint32_t)size);
}

static void jzon_static_deallocate(void* ptr)
{
	return static_allocator->deallocate(ptr);
}

namespace bowtie
{


const char* ResourceManager::resource_type_names[] = { "shader", "image", "texture", "font", "material" };

ResourceManager::ResourceManager(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface), _resources(hash::create<Resource>(allocator))
{
	static_allocator = &allocator;
	jzon_allocator.allocate = jzon_static_allocate;
	jzon_allocator.deallocate = jzon_static_deallocate;
}

ResourceManager::~ResourceManager()
{
	for(auto resource_iter = hash::begin(_resources); resource_iter != hash::end(_resources); ++resource_iter)
	{
		auto obj = resource_iter->value.object;
		switch(resource_iter->value.type)
		{
			case ResourceType::Material: _allocator.destroy((Material*)obj); break;
			case ResourceType::Shader: _allocator.destroy((Shader*)obj); break;
			case ResourceType::Image: _allocator.destroy((Image*)obj); break;
			case ResourceType::Texture: _allocator.destroy((Texture*)obj); break;
			case ResourceType::Font: _allocator.destroy((Font*)obj); break;
			default: assert(!"Some resource type isn't freed properly."); break;
		}
	}

	hash::deinit(_resources);
}

ResourceType ResourceManager::resource_type_from_string(const char* type)
{
	for (unsigned i = 0; i < (unsigned)ResourceType::NumResourceTypes; ++i)
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
	static const char* types_as_str[] = { "none", "mvp", "mv", "m", "time", "view_resolution", "view_resolution_ratio", "resolution" };
	
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
	auto existing = get(ResourceType::Material, name);

	if (existing.object != nullptr)
		return *(Material*)existing.object;

	auto material_file_option = file::load(filename, _allocator);
	assert(material_file_option.is_some && "Failed loading material.");
	auto& file = material_file_option.value;
	auto jzon_result = jzon_parse_custom_allocator((char*)file.data, &jzon_allocator); 
	assert(jzon_result.success && "Failed to parse font");
	_allocator.deallocate(file.data);

	auto jzon = jzon_result.output;
	auto shader_filename = jzon_get(jzon, "shader")->string_value;
	auto& shader = load_shader(shader_filename);
	auto uniforms_jzon = jzon_get(jzon, "uniforms");
	
	unsigned uniforms_size = sizeof(UniformResourceData) * uniforms_jzon->size;
	auto uniforms = (UniformResourceData*)_allocator.allocate(uniforms_size);
	Stream dynamic_uniform_data = {0};

	for(unsigned i = 0; i < uniforms_jzon->size; ++i)
	{
		auto uniform_json = uniforms_jzon->array_values[i];
		
		TempAllocator4096 ta;
		auto uniform_str = uniform_json->string_value;
		auto split_uniform = split(ta, uniform_str, ' ');
		assert(array::size(split_uniform) >= 2 && "Uniform definition must contain at least type and name.");
		auto type = get_uniform_type_from_str(split_uniform[0]);

		UniformResourceData uniform;
		uniform.type = type;
		uniform.name_offset = uniforms_size + dynamic_uniform_data.size;
		auto name = split_uniform[1];
		auto name_len = strlen32(name) + 1;
		stream::write(dynamic_uniform_data, name, name_len, _allocator);
		uniform.value_offset = (unsigned)-1;

		if (array::size(split_uniform) > 2)
		{
			auto value_str = split_uniform[2];
			uniform.automatic_value = get_automatic_value_from_str(value_str);
			
			if (uniform.automatic_value == uniform::None)
			{
				uniform.value_offset = uniforms_size + dynamic_uniform_data.size;

				switch (type)
				{
				case uniform::Float: {
					auto float_val = float_from_str(value_str);
					stream::write(dynamic_uniform_data, &float_val, sizeof(float), _allocator);
					} break;
				case uniform::Texture1:
				case uniform::Texture2:
				case uniform::Texture3:
					{
						auto texture = load_texture(value_str);	
						stream::write(dynamic_uniform_data, &texture.render_handle.handle, sizeof(unsigned), _allocator);
					}
					break;
				}
			}
		}

		array::deinit(split_uniform);
		uniforms[i] = uniform;
	}
	
	
	auto uniform_data_size = uniforms_size + dynamic_uniform_data.size;
	auto uniforms_data = _allocator.allocate(uniform_data_size);
	memcpy(uniforms_data, uniforms, uniforms_size);
	memcpy(memory::pointer_add(uniforms_data, uniforms_size), dynamic_uniform_data.start, dynamic_uniform_data.size);
	_allocator.deallocate(uniforms);
	_allocator.deallocate(dynamic_uniform_data.start);
	
	MaterialResourceData mrd;
	mrd.handle = _render_interface.create_handle();
	mrd.num_uniforms = uniforms_jzon->size;
	mrd.shader = shader.render_handle;
	RenderResourceData material_resource_data = _render_interface.create_render_resource_data(RenderResourceData::RenderMaterial);
	material_resource_data.data = &mrd;	
	_render_interface.create_resource(material_resource_data, uniforms_data, uniform_data_size);

	auto material = (Material*)_allocator.allocate(sizeof(Material));
	material->render_handle = mrd.handle;
	material->shader = &shader;
	add_resource(name, Resource(material));	
	jzon_free_custom_allocator(jzon, &jzon_allocator);
	return *material;
}

RenderResourceData get_create_render_resource_data(RenderInterface& render_interface, RenderResourceData::Type type, void* data)
{
	RenderResourceData resource_data = render_interface.create_render_resource_data(type);
	resource_data.data = data;
	return resource_data;
}

RenderResourceData get_update_render_resource_data(RenderResourceData::Type type, void* data)
{
	RenderResourceData resource_data = { type, data };
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
	auto shader_source_option = file::load(filename, allocator);
	assert(shader_source_option.is_some && "Failed loading shader source");
	auto& shader_source = shader_source_option.value;
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
	auto existing = get(ResourceType::Shader, name);

	if (existing.object != nullptr)
		return *(Shader*)existing.object;

	auto resource_package = get_shader_resource_data(_allocator, filename);
	resource_package.data.handle = _render_interface.create_handle();
	auto create_resource_data = get_create_render_resource_data(_render_interface, RenderResourceData::Shader, &resource_package.data);
	_render_interface.create_resource(create_resource_data, resource_package.dynamic_data, resource_package.dynamic_data_size);
	auto shader = (Shader*)_allocator.allocate(sizeof(Shader));
	shader->render_handle = resource_package.data.handle;
	add_resource(name, Resource(shader));
	return *shader;
}

Image& ResourceManager::load_image(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Image>(ResourceType::Image, name);
	if (existing != nullptr)
		return *existing;

	UncompressedTexture tex = png::load(filename, _allocator);
	
	auto image = _allocator.construct<Image>();
	image->resolution = Vector2u(tex.width, tex.height);
	image->data = tex.data;
	image->data_size = tex.data_size;
	image->pixel_format = PixelFormat::RGBA;
	
	add_resource(name, Resource(image));

	return *image;
}

Texture& ResourceManager::load_texture(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Texture>(ResourceType::Texture, name);
	if (existing != nullptr)
		return *existing;

	auto& image = load_image(filename);
	auto texture = (Texture*)_allocator.allocate(sizeof(Texture));
	texture->image = &image;
	texture->render_handle = RenderResourceHandle();
	_render_interface.create_texture(*texture);
	add_resource(name, Resource(texture));
	return *texture;	
}

Font& ResourceManager::load_font(const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get<Font>(ResourceType::Font, name);

	if (existing != nullptr)
		return *existing;
	
	auto font_option = file::load(filename, _allocator);
	assert(font_option.is_some && "Failed loading font");
	auto& file = font_option.value;
	auto jzon_result = jzon_parse_custom_allocator((char*)file.data, &jzon_allocator);
	assert(jzon_result.success && "Failed to parse font");
	_allocator.deallocate(file.data);
	auto jzon = jzon_result.output;
	auto texture_filename = jzon_get(jzon, "texture")->string_value;
	auto columns = jzon_get(jzon, "columns")->int_value;
	auto rows = jzon_get(jzon, "rows")->int_value;

	auto font = (Font*)_allocator.allocate(sizeof(Font));
	font->columns = columns;
	font->rows = rows;
	font->texture = &load_texture(texture_filename);
	add_resource(name, Resource(font));
	jzon_free_custom_allocator(jzon, &jzon_allocator);
	return *font;
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
		case ResourceType::Material: return Resource(&load_material(filename));
		case ResourceType::Image: return Resource(&load_image(filename));
		case ResourceType::Shader: return Resource(&load_shader(filename));
		case ResourceType::Texture: return Resource(&load_texture(filename));
		case ResourceType::Font: return Resource(&load_font(filename));
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
		case ResourceType::Shader:
			{
				auto& shader = *get<Shader>(type, filename);
				auto shader_data = get_shader_resource_data(_allocator, filename);
				shader_data.data.handle = shader.render_handle;
				auto update_command_data = get_update_render_resource_data(RenderResourceData::Shader, &shader_data.data);
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
	assert(_default_resources[(unsigned)type].object == 0 && "Trying to resassign already assigned default resource.");
	_default_resources[(unsigned)type] = resource;
}

Resource ResourceManager::get_default(ResourceType type) const
{
	auto resource = _default_resources[(unsigned)type];
	assert(resource.object != 0 && "No default resource set for this type.");
	return resource;
}

}
