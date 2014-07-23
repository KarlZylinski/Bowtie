#pragma once

namespace bowtie
{
	
struct RenderResourceHandle
{
	RenderResourceHandle() : type(NotInitialized) {}
	explicit RenderResourceHandle(unsigned h) : render_handle(h), type(Handle) {}
	explicit RenderResourceHandle(void* p) : render_object(p), type(Object) {}

	enum RenderResourceHandleType
	{
		NotInitialized, Handle, Object
	};

	RenderResourceHandleType type;
	union
	{
		void* render_object;
		unsigned render_handle;
	};
};

} // namespace bowtie