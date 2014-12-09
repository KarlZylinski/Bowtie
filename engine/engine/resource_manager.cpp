#include "resource_manager.h"

#include <cstring>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/murmur_hash.h>
#include <foundation/jzon.h>
#include <foundation/string_utils.h>
//#include <foundation/temp_allocator.h>
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
	return static_allocator->alloc_raw((uint32_t)size);
}

static void jzon_static_deallocate(void* ptr)
{
	return static_allocator->dealloc(ptr);
}

namespace bowtie
{

namespace internal
{

uint64_t hash_name(const char* name)
{
	return hash_str(name);
}

uint64_t get_name(uint64_t name, ResourceType type)
{
	char name_str[30];
	sprintf(name_str, "%u%llu", (unsigned)type, name);
	::uint64_t name_with_type;
	sscanf(name_str, "%llu", &name_with_type);
	return name_with_type;
}

Option<void*> get(const Hash<void*>& resources, ResourceType type, uint64_t name)
{
	return hash::try_get<void*>(resources, get_name(name, type));
}

void add(Hash<void*>& resources, uint64_t name, ResourceType type, void* resource)
{
	hash::set(resources, get_name(name, type), resource);
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

RenderResourceData get_create_render_resource_data(RenderResourceData::Type type, void* data)
{
	RenderResourceData resource_data = render_resource_data::create(type);
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
	allocator.dealloc(shader_source.data);

	ShaderResourceData srd;
	unsigned shader_dynamic_data_size = split_shader.vertex_source_len + split_shader.fragment_source_len;
	unsigned shader_dynamic_data_offset = 0;
	void* shader_resource_dynamic_data = allocator.alloc_raw(shader_dynamic_data_size);

	srd.vertex_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)shader_resource_dynamic_data, (char*)split_shader.vertex_source);
	shader_dynamic_data_offset += split_shader.vertex_source_len;

	srd.fragment_shader_source_offset = shader_dynamic_data_offset;
	strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), (char*)split_shader.fragment_source);

	allocator.dealloc(split_shader.vertex_source);
	allocator.dealloc(split_shader.fragment_source);
	return RenderResourcePackage<ShaderResourceData>(srd, shader_resource_dynamic_data, shader_dynamic_data_size);
}

Shader* load_shader(ResourceManager& rm, const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(rm._resources, ResourceType::Shader, name);

	if (existing.is_some)
		return (Shader*)existing.value;

	auto resource_package = get_shader_resource_data(*rm.allocator, filename);
	resource_package.data.handle = render_interface::create_handle(*rm.render_interface);
	auto create_resource_data = get_create_render_resource_data(RenderResourceData::Shader, &resource_package.data);
	render_interface::create_resource(*rm.render_interface, create_resource_data, resource_package.dynamic_data, resource_package.dynamic_data_size);
	auto shader = (Shader*)rm.allocator->alloc(sizeof(Shader));
	shader->render_handle = resource_package.data.handle;
	add(rm._resources, name, ResourceType::Shader, shader);
	return shader;
}

Image* load_image(ResourceManager& rm, const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(rm._resources, ResourceType::Image, name);

	if (existing.is_some)
		return (Image*)existing.value;

	UncompressedTexture tex = png::load(filename, *rm.allocator);
	auto image = (Image*)rm.allocator->alloc(sizeof(Image));
	image->resolution = Vector2u(tex.width, tex.height);
	image->data = tex.data;
	image->data_size = tex.data_size;
	image->pixel_format = PixelFormat::RGBA;
	add(rm._resources, name, ResourceType::Shader, image);
	return image;
}

Texture* load_texture(ResourceManager& rm, const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(rm._resources, ResourceType::Texture, name);

	if (existing.is_some)
		return (Texture*)existing.value;

	auto image = load_image(rm, filename);
	auto texture = (Texture*)rm.allocator->alloc(sizeof(Texture));
	texture->image = image;
	texture->render_handle = RenderResourceHandle();
	render_interface::create_texture(*rm.render_interface, *texture);
	add(rm._resources, name, ResourceType::Texture, texture);
	return texture;
}

Material* load_material(ResourceManager& rm, const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(rm._resources, ResourceType::Material, name);

	if (existing.is_some)
		return (Material*)existing.value;

	auto material_file_option = file::load(filename, *rm.allocator);
	assert(material_file_option.is_some && "Failed loading material.");
	auto& file = material_file_option.value;
	auto jzon_result = jzon_parse_custom_allocator((char*)file.data, &jzon_allocator);
	assert(jzon_result.success && "Failed to parse font");
	rm.allocator->dealloc(file.data);

	auto jzon = jzon_result.output;
	auto shader_filename = jzon_get(jzon, "shader")->string_value;
	auto shader = load_shader(rm, shader_filename);
	auto uniforms_jzon = jzon_get(jzon, "uniforms");

	unsigned uniforms_size = sizeof(UniformResourceData) * uniforms_jzon->size;
	auto uniforms = (UniformResourceData*)rm.allocator->alloc(uniforms_size);
	Stream dynamic_uniform_data = { 0 };

	for (unsigned i = 0; i < uniforms_jzon->size; ++i)
	{
		auto uniform_json = uniforms_jzon->array_values[i];
		auto uniform_str = uniform_json->string_value;
		auto split_uniform = split(*rm.allocator, uniform_str, ' ');
		assert(array::size(split_uniform) >= 2 && "Uniform definition must contain at least type and name.");
		auto type = get_uniform_type_from_str(split_uniform[0]);

		UniformResourceData uniform;
		uniform.type = type;
		uniform.name_offset = uniforms_size + dynamic_uniform_data.size;
		auto name = split_uniform[1];
		auto name_len = strlen32(name) + 1;
		stream::write(dynamic_uniform_data, name, name_len, *rm.allocator);
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
					stream::write(dynamic_uniform_data, &float_val, sizeof(float), *rm.allocator);
				} break;
				case uniform::Texture1:
				case uniform::Texture2:
				case uniform::Texture3:
				{
					auto texture = load_texture(rm, value_str);
					stream::write(dynamic_uniform_data, &texture->render_handle.handle, sizeof(unsigned), *rm.allocator);
				}
					break;
				}
			}
		}

		for (unsigned j = 0; j < array::size(split_uniform); ++j)
			rm.allocator->dealloc(split_uniform[j]);

		array::deinit(split_uniform);
		uniforms[i] = uniform;
	}


	auto uniform_data_size = uniforms_size + dynamic_uniform_data.size;
	auto uniforms_data = rm.allocator->alloc_raw(uniform_data_size);
	memcpy(uniforms_data, uniforms, uniforms_size);
	memcpy(memory::pointer_add(uniforms_data, uniforms_size), dynamic_uniform_data.start, dynamic_uniform_data.size);
	rm.allocator->dealloc(uniforms);
	rm.allocator->dealloc(dynamic_uniform_data.start);

	MaterialResourceData mrd;
	mrd.handle = render_interface::create_handle(*rm.render_interface);
	mrd.num_uniforms = uniforms_jzon->size;
	mrd.shader = shader->render_handle;
	RenderResourceData material_resource_data = render_resource_data::create(RenderResourceData::RenderMaterial);
	material_resource_data.data = &mrd;
	render_interface::create_resource(*rm.render_interface, material_resource_data, uniforms_data, uniform_data_size);

	auto material = (Material*)rm.allocator->alloc(sizeof(Material));
	material->render_handle = mrd.handle;
	material->shader = shader;
	add(rm._resources, name, ResourceType::Material, material);
	jzon_free_custom_allocator(jzon, &jzon_allocator);
	return material;
}

Font* load_font(ResourceManager& rm, const char* filename)
{
	auto name = hash_name(filename);
	auto existing = get(rm._resources, ResourceType::Font, name);
	
	if (existing.is_some)
		return (Font*)existing.value;

	auto font_option = file::load(filename, *rm.allocator);
	assert(font_option.is_some && "Failed loading font");
	auto& file = font_option.value;
	auto jzon_result = jzon_parse_custom_allocator((char*)file.data, &jzon_allocator);
	assert(jzon_result.success && "Failed to parse font");
	rm.allocator->dealloc(file.data);
	auto jzon = jzon_result.output;
	auto texture_filename = jzon_get(jzon, "texture")->string_value;
	auto columns = jzon_get(jzon, "columns")->int_value;
	auto rows = jzon_get(jzon, "rows")->int_value;

	auto font = (Font*)rm.allocator->alloc(sizeof(Font));
	font->columns = columns;
	font->rows = rows;
	font->texture = load_texture(rm, texture_filename);
	add(rm._resources, name, ResourceType::Font, font);
	jzon_free_custom_allocator(jzon, &jzon_allocator);
	return font;
}


} // namespace internal

namespace resource_manager
{

ResourceType resource_type_from_string(const char* type)
{
	for (unsigned i = 0; i < (unsigned)ResourceType::NumResourceTypes; ++i)
	{
		if (strequal(type, resource_type_names[i]))
			return (ResourceType)i;
	}

	assert(!"Unknown resource type string");
	return ResourceType::NumResourceTypes;
}

void init(ResourceManager& rm, Allocator& allocator, RenderInterface& render_interface)
{
	rm.allocator = &allocator;
	rm.render_interface = &render_interface;
	memset(rm._default_resources, 0, sizeof(Option<void*>) * (unsigned)ResourceType::NumResourceTypes);
	hash::init<void*>(rm._resources, *rm.allocator);
	static_allocator = &allocator;
	jzon_allocator.allocate = jzon_static_allocate;
	jzon_allocator.deallocate = jzon_static_deallocate;
}

void deinit(ResourceManager& rm)
{
	for(auto resource_iter = hash::begin(rm._resources); resource_iter != hash::end(rm._resources); ++resource_iter)
		rm.allocator->dealloc(resource_iter->value);

	hash::deinit(rm._resources);
}

Option<void*> get(const ResourceManager& rm, ResourceType type, uint64_t name)
{
	return internal::get(rm._resources, type, name);
}

Option<void*> load(ResourceManager& rm, ResourceType type, const char* filename)
{
	switch(type)
	{
		case ResourceType::Material: return option::some<void*>(internal::load_material(rm, filename));
		case ResourceType::Image: return option::some<void*>(internal::load_image(rm, filename));
		case ResourceType::Shader: return option::some<void*>(internal::load_shader(rm, filename));
		case ResourceType::Texture: return option::some<void*>(internal::load_texture(rm, filename));
		case ResourceType::Font: return option::some<void*>(internal::load_font(rm, filename));
		default: return option::none<void*>();
	}
}

void reload(ResourceManager& rm, ResourceType type, const char* filename)
{
	auto name = internal::hash_name(filename);

	switch(type)
	{
		case ResourceType::Shader:
			{
				auto shader = (Shader*)option::get(get(rm, type, name));
				auto shader_data = internal::get_shader_resource_data(*rm.allocator, filename);
				shader_data.data.handle = shader->render_handle;
				auto update_command_data = internal::get_update_render_resource_data(RenderResourceData::Shader, &shader_data.data);
				render_interface::update_resource(*rm.render_interface, update_command_data, shader_data.dynamic_data, shader_data.dynamic_data_size);
			}
			break;
		default:
			assert(!"Tried to reload unsupported resource type");
			break;
	}
}

void reload_all(ResourceManager&)
{
	// TODO: Find all resources through FS thingy and send them off to reload.
}

void set_default(ResourceManager& rm, ResourceType type, void* resource)
{
	rm._default_resources[(unsigned)type] = option::some(resource);
}

Option<void*> get_default(const ResourceManager& rm, ResourceType type)
{
	return rm._default_resources[(unsigned)type];
}

} // namespace resource_manager

} // namespace bowtie
