#pragma once

namespace bowtie
{

struct CapturedCallstack
{
	unsigned short num_frames;
	void* frames[64];
	void* ptr;
	bool used;
};

}
