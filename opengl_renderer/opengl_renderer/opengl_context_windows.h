#pragma once

#include "opengl_context.h"
#include <Windows.h>

namespace bowtie
{

class OpenGLContextWindows : public OpenGLContext
{
public:
	void create(HWND window);
	virtual void flip();
	virtual void make_current_for_calling_thread();
private:
	HDC _device_context;
	HGLRC _rendering_context;
};

}

