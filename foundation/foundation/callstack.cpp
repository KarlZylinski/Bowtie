#include "callstack.h"

#if defined(_WIN32)
	#include <Windows.h>
#endif

namespace bowtie
{
namespace callstack
{

CapturedCallstack capture()
{
	#if defined(_WIN32)
		const unsigned long frames_to_skip = 0;
		const unsigned long frames_to_capture = 64;
		unsigned long back_trace_hash = 0; 
		CapturedCallstack cc;
		cc.num_frames = CaptureStackBackTrace(frames_to_skip, frames_to_capture, cc.frames, &back_trace_hash);
		return cc;
	#endif
}

}
}
