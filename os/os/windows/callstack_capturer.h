#pragma once

#include <foundation/icallstack_capturer.h>

namespace bowtie
{

class CallstackCapturer : public ICallstackCapturer
{
public:
	CapturedCallstack capture(unsigned frames_to_skip);
	void print_callstack(const char* caption, const CapturedCallstack& captured_callstack);
};

} // namespace bowtie