#pragma once

namespace bowtie
{

struct CapturedCallstack;
struct CallstackCapturer;

namespace allocator_helpers
{

void add_captured_callstack(CapturedCallstack* callstacks, const CapturedCallstack& cc);
void remove_captured_callstack(CapturedCallstack* callstacks, void* p);
void ensure_captured_callstacks_unused(CallstackCapturer* callstack_capturer, CapturedCallstack* callstacks);

}

}