#pragma once

namespace bowtie
{

struct RenderResourceHandle
{
	static const unsigned NotInitialized = ((unsigned)-1);

	RenderResourceHandle() : handle(NotInitialized) {}
	RenderResourceHandle(unsigned handle) : handle(handle) {}

	bool operator==(const RenderResourceHandle& other) const
	{
		return handle == other.handle;
	}

	bool operator==(const unsigned other_handle) const
	{
		return handle == other_handle;
	}
	
	bool operator!=(const RenderResourceHandle& other) const
	{
		return handle != other.handle;
	}

	bool operator!=(const unsigned other_handle) const
	{
		return handle != other_handle;
	}
	
	bool operator<(const RenderResourceHandle& other) const
	{
		return handle < other.handle;
	}

	bool operator<(const unsigned other_handle) const
	{
		return handle < other_handle;
	}

	unsigned handle;
};

} // namespace bowtie