#pragma once

namespace bowtie
{
    
enum class RenderResourceType
{
    NotInitialized, Handle, Object
};

struct RenderResource
{
    RenderResourceType type;
    union
    {
        void* object;
        uint32 handle;
    };
};

namespace render_resource
{
    inline RenderResource create_handle(uint32 h)
    {
        RenderResource rr;
        rr.type = RenderResourceType::Handle;
        rr.handle = h;
        return rr;
    }

    inline RenderResource create_object(void* p)
    {
        RenderResource rr;
        rr.type = RenderResourceType::Object;
        rr.object = p;
        return rr;
    }
}

} // namespace bowtie