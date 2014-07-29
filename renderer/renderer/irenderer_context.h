#pragma once

namespace bowtie
{

class IRendererContext
{
public:
	virtual ~IRendererContext() {}
	virtual void flip() = 0;
	virtual void make_current_for_calling_thread() = 0;
};

}