#pragma once

namespace bowtie
{

struct ResourceHandle
{
	ResourceHandle() : type(NotInitialized) {}
	ResourceHandle(unsigned h) : handle(h), type(Handle) {}
	ResourceHandle(void* p) : object(p), type(Object) {}

	enum ResourceHandleType
	{
		NotInitialized, Handle, Object
	};

	ResourceHandleType type;
	union
	{
		void* object;
		unsigned handle;
	};
};

} // namespace bowtie