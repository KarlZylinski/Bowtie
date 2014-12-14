#include "concurrent_ring_buffer.h"
#include <base/malloc_allocator.h>
#include <os/windows/callstack_capturer.h>
#include <Windows.h>

using namespace bowtie;

void single_temp_alloc()
{
    auto r = (rand() % 16) + 16;
    auto p = temp_memory::alloc(r);
    *(uint16*)p = rand() % 32;
}

void test_temp_memory()
{
    for (unsigned i = 0; i < 20000; ++i)
    {
        temp_memory::new_frame();
        single_temp_alloc();
        single_temp_alloc();
        single_temp_alloc();
    }
}

PermanentMemory bowtie::MainThreadMemory;
PermanentMemory bowtie::RenderThreadMemory;

int WINAPI WinMain(__in HINSTANCE, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
    {
        const auto temp_memory_size = 256u;
        void* temp_memory_buffer = VirtualAlloc(0, temp_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        temp_memory::init(temp_memory_buffer, temp_memory_size);
        test_temp_memory();
        VirtualFree(temp_memory_buffer, 0, MEM_RELEASE);
    }
}
