#include "concurrent_ring_buffer.h"
#include "vector.h"
#include <foundation/malloc_allocator.h>
#include <os/windows/callstack_capturer.h>
#include <Windows.h>

using namespace bowtie;

int WINAPI WinMain(__in HINSTANCE, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	auto callstack_capturer = callstack_capturer::create();
	auto& allocator = *(new MallocAllocator());
	memory::init_allocator(allocator, "default allocator", &callstack_capturer);
	//tests::test_concurrent_ring_buffer(allocator);
	tests::test_vector(&allocator);
	memory::deinit_allocator(allocator);
}
