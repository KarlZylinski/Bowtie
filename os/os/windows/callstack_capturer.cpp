#include "callstack_capturer.h"

#if defined(_WIN32)
	#include <Windows.h>
	#include <DbgHelp.h>
#endif

namespace bowtie
{

namespace internal
{

CapturedCallstack capture(unsigned frames_to_skip)
{
	#if defined(_WIN32)
		const unsigned long frames_to_capture = 64;
		unsigned long back_trace_hash = 0; 
		CapturedCallstack cc;
		cc.num_frames = CaptureStackBackTrace(frames_to_skip + 2, frames_to_capture, cc.frames, &back_trace_hash);
		return cc;
	#endif
}	

void print_callstack(const char* caption, const CapturedCallstack& captured_callstack)
{
	#if defined(_WIN32)
		HANDLE process = GetCurrentProcess();
		SymInitialize(process, NULL, TRUE);
		SYMBOL_INFO* symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		char* callstack_str = (char*)malloc(symbol->MaxNameLen * 64);
		unsigned callstack_str_size = 0;
		for (unsigned i = 0; i < captured_callstack.num_frames; i++ )
		{
			SymFromAddr(process, (DWORD64)(captured_callstack.frames[i]), 0, symbol);
			memcpy(callstack_str + callstack_str_size, symbol->Name, symbol->NameLen);
			callstack_str[callstack_str_size + symbol->NameLen] = '\n';
			callstack_str_size += symbol->NameLen + 1;
		}
		callstack_str[callstack_str_size] = 0;
		MessageBox(nullptr, callstack_str, caption, MB_ICONERROR);
	#endif
}

} // namespace internal


namespace callstack_capturer
{

CallstackCapturer create()
{
	CallstackCapturer cc;
	cc.capture = &internal::capture;
	cc.print_callstack = &internal::print_callstack;
	return cc;
}

} // namespace captured_callstack

} //namespace bowtie