#pragma once

namespace bowtie
{

struct CapturedCallstack
{
	unsigned short num_frames;
	void* frames[64];
};

namespace callstack
{

CapturedCallstack capture();
void print_callstack(const char* caption, const CapturedCallstack& captured_callstack);

}
}
