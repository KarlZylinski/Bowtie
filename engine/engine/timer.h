#pragma once

namespace bowtie
{

struct Timer
{
	void (*start)();
	float (*counter)();
};

}
