#pragma once
#include <base/hash.h>
#include <base/murmur_hash.h>
#include <base/string_utils.h>
#include "renderer/render_resource_handle.h"
#include "resource_type.h"

namespace bowtie
{
struct Material;
struct RenderInterface;
struct Texture;
struct Image;
struct Shader;
struct Font;

struct ResourceStore
{
    Allocator* allocator;
    RenderInterface* render_interface;
    Hash<void*> _resources;
    Option<void*> _default_resources[(uint32)ResourceType::NumResourceTypes];
};

namespace resource_store
{
    static const char* resource_type_names[] = { "not_initialized", "shader", "image", "texture", "font", "material" };
    ResourceType resource_type_from_string(const char* type);

    void init(ResourceStore* rs, Allocator* allocator, RenderInterface* render_interface);
    void deinit(ResourceStore* rs);
    Option<void*> load(ResourceStore* rs, ResourceType type, const char* filename);
    Option<void*> get(const ResourceStore* rs, ResourceType type, uint64 name);
    void reload(ResourceStore* rs, ResourceType type, const char* filename);
    void reload_all(ResourceStore* rs);
    void set_default(ResourceStore* rs, ResourceType type, void* resource);
    Option<void*> get_default(const ResourceStore* rs, ResourceType type);
}

} // namespace bowtie