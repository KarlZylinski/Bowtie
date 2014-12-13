
#ifdef _WIN32
#include "timer.h"

#include <Windows.h>

namespace
{
	double frequency = 0.0;
	long long counter_start = 0;
}

namespace bowtie
{
namespace windows
{
namespace timer
{

void start()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	frequency = double(li.QuadPart)/1000.0;

	QueryPerformanceCounter(&li);
	counter_start = li.QuadPart;
}

float counter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return float(double(li.QuadPart - counter_start) / frequency / 1000.0);
}

} // namespace timer
} // namespace windows
} // namespace bowtie

#endif