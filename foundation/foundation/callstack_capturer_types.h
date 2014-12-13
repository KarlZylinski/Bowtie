#pragma once

namespace bowtie
{

struct CapturedCallstack
{
	uint16 num_frames;
	void* frames[64];
	void* ptr;
	bool used;
};

}
