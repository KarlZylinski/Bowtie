#include "resource_store.h"

#include <cstring>

#include <base/file.h>
#include <base/memory.h>
#include <base/murmur_hash.h>
#include <base/jzon.h>
#include <base/string_utils.h>
#include <base/stream.h>
#include <resource_path.h>

#include "font.h"
#include "material.h"
#include "png.h"
#include "render_interface.h"
#include "shader.h"
#include "shader_utils.h"
#include "texture.h"

static JzonAllocator jzon_allocator;

static void* jzon_static_allocate(size_t size)
{
    return bowtie::temp_memory::alloc_raw(size);
}

// Allocator uses temp memory, does nothing.
static void jzon_static_deallocate(void*)
{
}

namespace bowtie
{

namespace internal
{

uint64 hash_name(const char* name)
{
    return hash_str(name);
}

uint64 get_name(uint64 name, ResourceType type)
{
    char name_str[30];
    sprintf(name_str, "%u%llu", (uint32)type, name);
    uint64 name_with_type;
    sscanf(name_str, "%llu", &name_with_type);
    return name_with_type;
}

Option<void*> get(const Hash<void*>* resources, ResourceType type, uint64 name)
{
    return hash::try_get<void*>(resources, get_name(name, type));
}

void add(Hash<void*>* resources, uint64 name, ResourceType type, void* resource)
{
    hash::set(resources, get_name(name, type), resource);
}

uniform::Type get_uniform_type_from_str(const char* str)
{
    static const char* types_as_str[] = { "float", "vec2", "vec3", "vec4", "mat3", "mat4", "texture1", "texture2", "texture3" };

    for (uint32 i = 0; i < uniform::NumUniformTypes; ++i)
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

    for (uint32 i = 0; i < uniform::NumAutomaticValues; ++i)
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
    RenderResourcePackage(const T& data, void* dynamic_data, uint32 dynamic_data_size)
        : data(data), dynamic_data(dynamic_data), dynamic_data_size(dynamic_data_size)
    {}

    T data;
    void* dynamic_data;
    uint32 dynamic_data_size;
};

RenderResourcePackage<ShaderResourceData> get_shader_resource_data(const char* filename)
{
    auto shader_source_option = file::load(filename);
    assert(shader_source_option.is_some && "Failed loading shader source");
    auto shader_source = &shader_source_option.value;
    auto split_shader = shader_utils::split_shader(shader_source);

    ShaderResourceData srd;
    uint32 shader_dynamic_data_size = split_shader.vertex_source_len + split_shader.fragment_source_len;
    uint32 shader_dynamic_data_offset = 0;
    void* shader_resource_dynamic_data = temp_memory::alloc_raw(shader_dynamic_data_size);

    srd.vertex_shader_source_offset = shader_dynamic_data_offset;
    strcpy((char*)shader_resource_dynamic_data, (char*)split_shader.vertex_source);
    shader_dynamic_data_offset += split_shader.vertex_source_len;

    srd.fragment_shader_source_offset = shader_dynamic_data_offset;
    strcpy((char*)memory::pointer_add(shader_resource_dynamic_data, shader_dynamic_data_offset), (char*)split_shader.fragment_source);
    
    return RenderResourcePackage<ShaderResourceData>(srd, shader_resource_dynamic_data, shader_dynamic_data_size);
}

Shader* load_shader(ResourceStore* rs, const char* filename)
{
    auto name = hash_name(filename);
    auto existing = get(&rs->_resources, ResourceType::Shader, name);

    if (existing.is_some)
        return (Shader*)existing.value;

    auto resource_package = get_shader_resource_data(filename);
    resource_package.data.handle = render_interface::create_handle(rs->render_interface);
    auto create_resource_data = get_create_render_resource_data(RenderResourceData::Shader, &resource_package.data);
    render_interface::create_resource(rs->render_interface, &create_resource_data, resource_package.dynamic_data, resource_package.dynamic_data_size);
    auto shader = (Shader*)debug_memory::alloc(sizeof(Shader));
    shader->render_handle = resource_package.data.handle;
    add(&rs->_resources, name, ResourceType::Shader, shader);
    return shader;
}

Image* load_image(ResourceStore* rs, const char* filename)
{
    auto name = hash_name(filename);
    auto existing = get(&rs->_resources, ResourceType::Image, name);

    if (existing.is_some)
        return (Image*)existing.value;

    UncompressedTexture tex = png::load(filename);
    auto image = (Image*)debug_memory::alloc(sizeof(Image));
    image->resolution = vector2u::create(tex.width, tex.height);
    image->data = tex.data;
    image->data_size = tex.data_size;
    image->pixel_format = PixelFormat::RGBA;
    add(&rs->_resources, name, ResourceType::Shader, image);
    return image;
}

Texture* load_texture(ResourceStore* rs, const char* filename)
{
    auto name = hash_name(filename);
    auto existing = get(&rs->_resources, ResourceType::Texture, name);

    if (existing.is_some)
        return (Texture*)existing.value;

    auto image = load_image(rs, filename);
    auto texture = (Texture*)debug_memory::alloc(sizeof(Texture));
    texture->image = image;
    texture->render_handle = RenderResourceHandle();
    render_interface::create_texture(rs->render_interface, texture);
    add(&rs->_resources, name, ResourceType::Texture, texture);
    return texture;
}

Material* load_material(ResourceStore* rs, const char* filename)
{
    auto name = hash_name(filename);
    auto existing = get(&rs->_resources, ResourceType::Material, name);

    if (existing.is_some)
        return (Material*)existing.value;

    auto material_file_option = file::load(filename);
    assert(material_file_option.is_some && "Failed loading material.");
    auto file = &material_file_option.value;
    auto jzon_result = jzon_parse_custom_allocator((char*)file->data, &jzon_allocator);
    assert(jzon_result.success && "Failed to parse font");

    auto jzon = jzon_result.output;
    auto shader_filename = jzon_get(jzon, "shader")->string_value;
    auto shader = load_shader(rs, shader_filename);
    auto uniforms_jzon = jzon_get(jzon, "uniforms");

    uint32 uniforms_size = sizeof(UniformResourceData) * uniforms_jzon->size;
    auto uniforms = (UniformResourceData*)temp_memory::alloc(uniforms_size);
    Stream dynamic_uniform_data = {};

    for (uint32 i = 0; i < uniforms_jzon->size; ++i)
    {
        auto uniform_json = uniforms_jzon->array_values[i];
        auto uniform_str = uniform_json->string_value;
        auto split_uniform = split(uniform_str, ' ');
        assert(split_uniform.size >= 2 && "Uniform definition must contain at least type and name.");
        auto type = get_uniform_type_from_str(split_uniform[0]);

        UniformResourceData uniform;
        uniform.type = type;
        uniform.name_offset = uniforms_size + dynamic_uniform_data.size;
        auto name = split_uniform[1];
        auto name_len = strlen32(name) + 1;
        stream::write(&dynamic_uniform_data, name, name_len);
        uniform.value_offset = (uint32)-1;

        if (split_uniform.size > 2)
        {
            auto value_str = split_uniform[2];
            uniform.automatic_value = get_automatic_value_from_str(value_str);

            if (uniform.automatic_value == uniform::None)
            {
                uniform.value_offset = uniforms_size + dynamic_uniform_data.size;

                switch (type)
                {
                case uniform::Float: {
                    auto real32_val = real32_from_str(value_str);
                    stream::write(&dynamic_uniform_data, &real32_val, sizeof(real32));
                } break;
                case uniform::Texture1:
                case uniform::Texture2:
                case uniform::Texture3:
                {
                    auto texture = load_texture(rs, value_str);
                    stream::write(&dynamic_uniform_data, &texture->render_handle, sizeof(uint32));
                }
                    break;
                }
            }
        }

        uniforms[i] = uniform;
    }

    auto uniform_data_size = uniforms_size + dynamic_uniform_data.size;
    auto uniforms_data = temp_memory::alloc_raw(uniform_data_size);
    memcpy(uniforms_data, uniforms, uniforms_size);
    memcpy(memory::pointer_add(uniforms_data, uniforms_size), dynamic_uniform_data.start, dynamic_uniform_data.size);

    MaterialResourceData mrd;
    mrd.handle = render_interface::create_handle(rs->render_interface);
    mrd.num_uniforms = uniforms_jzon->size;
    mrd.shader = shader->render_handle;
    RenderResourceData material_resource_data = render_resource_data::create(RenderResourceData::RenderMaterial);
    material_resource_data.data = &mrd;
    render_interface::create_resource(rs->render_interface, &material_resource_data, uniforms_data, uniform_data_size);

    auto material = (Material*)debug_memory::alloc(sizeof(Material));
    material->render_handle = mrd.handle;
    material->shader = shader;
    add(&rs->_resources, name, ResourceType::Material, material);
    jzon_free_custom_allocator(jzon, &jzon_allocator);
    return material;
}

Font* load_font(ResourceStore* rs, const char* filename)
{
    auto name = hash_name(filename);
    auto existing = get(&rs->_resources, ResourceType::Font, name);
    
    if (existing.is_some)
        return (Font*)existing.value;

    auto font_option = file::load(filename);
    assert(font_option.is_some && "Failed loading font");
    auto file = &font_option.value;
    auto jzon_result = jzon_parse_custom_allocator((char*)file->data, &jzon_allocator);
    assert(jzon_result.success && "Failed to parse font");
    auto jzon = jzon_result.output;
    auto texture_filename = jzon_get(jzon, "texture")->string_value;
    auto columns = jzon_get(jzon, "columns")->int_value;
    auto rows = jzon_get(jzon, "rows")->int_value;

    auto font = (Font*)debug_memory::alloc(sizeof(Font));
    font->columns = columns;
    font->rows = rows;
    font->texture = load_texture(rs, texture_filename);
    add(&rs->_resources, name, ResourceType::Font, font);
    jzon_free_custom_allocator(jzon, &jzon_allocator);
    return font;
}


} // namespace internal

namespace resource_store
{

ResourceType resource_type_from_string(const char* type)
{
    for (uint32 i = 0; i < (uint32)ResourceType::NumResourceTypes; ++i)
    {
        if (strequal(type, resource_type_names[i]))
            return (ResourceType)i;
    }

    assert(!"Unknown resource type string");
    return ResourceType::NumResourceTypes;
}

void init(ResourceStore* rs, Allocator* allocator, RenderInterface* render_interface)
{
    rs->allocator = allocator;
    rs->render_interface = render_interface;
    memset(rs->_default_resources, 0, sizeof(Option<void*>) * (uint32)ResourceType::NumResourceTypes);
    hash::init<void*>(&rs->_resources, rs->allocator);
    jzon_allocator.allocate = jzon_static_allocate;
    jzon_allocator.deallocate = jzon_static_deallocate;
}

void deinit(ResourceStore* rs)
{
    for(auto resource_iter = hash::begin(&rs->_resources); resource_iter != hash::end(&rs->_resources); ++resource_iter)
        debug_memory::dealloc(resource_iter->value);

    hash::deinit(&rs->_resources);
}

Option<void*> get(const ResourceStore* rs, ResourceType type, uint64 name)
{
    return internal::get(&rs->_resources, type, name);
}

Option<void*> load(ResourceStore* rs, ResourceType type, const char* filename)
{
    switch(type)
    {
        case ResourceType::Material: return option::some<void*>(internal::load_material(rs, filename));
        case ResourceType::Image: return option::some<void*>(internal::load_image(rs, filename));
        case ResourceType::Shader: return option::some<void*>(internal::load_shader(rs, filename));
        case ResourceType::Texture: return option::some<void*>(internal::load_texture(rs, filename));
        case ResourceType::Font: return option::some<void*>(internal::load_font(rs, filename));
        default: return option::none<void*>();
    }
}

void reload(ResourceStore* rs, ResourceType type, const char* filename)
{
    auto name = internal::hash_name(filename);

    switch(type)
    {
        case ResourceType::Shader:
            {
                auto shader = (Shader*)option::get(get(rs, type, name));
                auto shader_data = internal::get_shader_resource_data(filename);
                shader_data.data.handle = shader->render_handle;
                auto update_command_data = internal::get_update_render_resource_data(RenderResourceData::Shader, &shader_data.data);
                render_interface::update_resource(rs->render_interface, &update_command_data, shader_data.dynamic_data, shader_data.dynamic_data_size);
            }
            break;
        default:
            assert(!"Tried to reload unsupported resource type");
            break;
    }
}

void reload_all(ResourceStore*)
{
    // TODO: Find all resources through FS thingy and send them off to reload.
}

void set_default(ResourceStore* rs, ResourceType type, void* resource)
{
    rs->_default_resources[(uint32)type] = option::some(resource);
}

Option<void*> get_default(const ResourceStore* rs, ResourceType type)
{
    return rs->_default_resources[(uint32)type];
}

} // namespace resource_store

} // namespace bowtie
