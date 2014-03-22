#pragma once

namespace bowtie
{
	
struct InternalRenderResourceHandle
{
	InternalRenderResourceHandle() : type(NotInitialized) {}
	InternalRenderResourceHandle(unsigned h) : handle(h), type(Handle) {}
	InternalRenderResourceHandle(void* p) : object(p), type(Object) {}

	enum InternalRenderResourceHandleType
	{
		Handle, Object, NotInitialized
	};

	InternalRenderResourceHandleType type;
	union
	{
		void* object;
		unsigned handle;
	};
};

} // namespace bowtie