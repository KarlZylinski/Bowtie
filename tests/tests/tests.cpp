#include "concurrent_ring_buffer.h"
#include <foundation/memory.h>
#include <os/windows/callstack_capturer.h>
#include <Windows.h>

using namespace bowtie;

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	auto callstack_capturer = new CallstackCapturer();
	memory_globals::init(*callstack_capturer);
	Allocator& allocator = memory_globals::default_allocator();
	tests::test_concurrent_ring_buffer(allocator);
	memory_globals::shutdown();
	delete callstack_capturer;
}
