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
		Handle, Object, NotInitialized
	};

	ResourceHandleType type;
	union
	{
		void* object;
		unsigned handle;
	};
};

} // namespace bowtie