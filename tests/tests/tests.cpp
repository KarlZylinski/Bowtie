#include "concurrent_ring_buffer.h"
#include <foundation/malloc_allocator.h>
#include <os/windows/callstack_capturer.h>
#include <Windows.h>

using namespace bowtie;

int WINAPI WinMain(__in HINSTANCE, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	auto callstack_capturer = windows::callstack_capturer::create();
	MallocAllocator allocator;
	memory::init_allocator(&allocator, "default allocator", &callstack_capturer);
	tests::test_concurrent_ring_buffer(&allocator);
	memory::deinit_allocator(&allocator);
}
