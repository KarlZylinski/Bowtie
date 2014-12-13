#pragma once

namespace bowtie
{

struct PlatformRendererContextData;

struct RendererContext
{
	void (*flip)(PlatformRendererContextData* c);
	void (*make_current_for_calling_thread)(PlatformRendererContextData* c);
};

}