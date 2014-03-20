#pragma once

#include <condition_variable>
#include <mutex>

namespace bowtie
{

struct RenderFence	
{
	std::condition_variable fence_processed;
	std::mutex mutex;
	bool processed;
	
	RenderFence()
	{
		processed = false;
	}

	RenderFence(const RenderFence&)
	{
		processed = false;
	}
};

}