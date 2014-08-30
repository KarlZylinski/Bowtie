#pragma once

namespace bowtie
{
	
struct RenderResource
{
	RenderResource() : type(NotInitialized) {}
	explicit RenderResource(unsigned h) : render_handle(h), type(Handle) {}
	explicit RenderResource(void* p) : render_object(p), type(Object) {}

	enum RenderResourceType
	{
		NotInitialized, Handle, Object
	};

	RenderResourceType type;
	union
	{
		void* render_object;
		unsigned render_handle;
	};
};

} // namespace bowtie