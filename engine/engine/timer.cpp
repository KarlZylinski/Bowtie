#include "timer.h"

#ifdef _WIN32
	#include <Windows.h>

namespace
{
	double frequency = 0.0;
	long long counter_start = 0;
}
#endif

namespace bowtie
{
namespace timer
{

void start()
{
	#ifdef _WIN32
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		frequency = double(li.QuadPart)/1000.0;

		QueryPerformanceCounter(&li);
		counter_start = li.QuadPart;
	#endif
}

float counter()
{
	#ifdef _WIN32
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return float(double(li.QuadPart - counter_start) / frequency / 1000.0);
	#endif
}

} // namespace timer
} // namespace bowtie
