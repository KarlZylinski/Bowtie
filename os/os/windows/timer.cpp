
#ifdef _WIN32
#include "timer.h"

#include <Windows.h>

namespace bowtie
{

namespace
{
    real64 frequency = 0.0;
    long long counter_start = 0;
}

namespace windows
{
namespace timer
{

void start()
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    frequency = real64(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    counter_start = li.QuadPart;
}

real32 counter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return real32(real64(li.QuadPart - counter_start) / frequency / 1000.0);
}

} // namespace timer
} // namespace windows
} // namespace bowtie

#endif