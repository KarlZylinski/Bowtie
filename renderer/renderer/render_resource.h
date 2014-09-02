#pragma once

namespace bowtie
{
	
struct RenderResource
{
	RenderResource() : type(NotInitialized) {}
	explicit RenderResource(unsigned h) : handle(h), type(Handle) {}
	explicit RenderResource(void* p) : object(p), type(Object) {}

	enum RenderResourceType
	{
		NotInitialized, Handle, Object
	};

	RenderResourceType type;
	union
	{
		void* object;
		unsigned handle;
	};
};

} // namespace bowtie