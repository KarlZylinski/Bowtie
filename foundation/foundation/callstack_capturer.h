#pragma once

namespace bowtie
{

struct CapturedCallstack
{
	unsigned short num_frames;
	void* frames[64];
};

struct CallstackCapturer
{
	CapturedCallstack(*capture)(unsigned frames_to_skip);
	void (*print_callstack)(const char* caption, const CapturedCallstack& captured_callstack);
};

}
