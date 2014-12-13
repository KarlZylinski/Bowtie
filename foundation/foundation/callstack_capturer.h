#pragma once

#include "callstack_capturer_types.h"

namespace bowtie
{

struct CallstackCapturer
{
	CapturedCallstack(*capture)(uint32 frames_to_skip, void* p);
	void (*print_callstack)(const char* caption, const CapturedCallstack* captured_callstack);
};

}
