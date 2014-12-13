#pragma once

#include <os/renderer_context.h>
#include <Windows.h>

namespace bowtie
{

struct PlatformRendererContextData
{
	HDC device_context;
	HGLRC rendering_context;
};

namespace windows
{

namespace opengl_context
{
	void init(PlatformRendererContextData* context, HWND hwnd);
	bowtie::RendererContext create();
}

}

}
