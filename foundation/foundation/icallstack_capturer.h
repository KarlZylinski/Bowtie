#pragma once

namespace bowtie
{

struct CapturedCallstack
{
	unsigned short num_frames;
	void* frames[64];
};

class ICallstackCapturer {
public:
	virtual ~ICallstackCapturer() {}
	virtual CapturedCallstack capture(unsigned frames_to_skip) = 0;
	virtual void print_callstack(const char* caption, const CapturedCallstack& captured_callstack) = 0;
};

}
