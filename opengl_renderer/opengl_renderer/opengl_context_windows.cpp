#include "opengl_context_windows.h"
#include "gl3w.h"

namespace bowtie
{

void OpenGLContextWindows::create(HWND window)
{
	PIXELFORMATDESCRIPTOR pfd =
    {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                        //Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		32,                        //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
    };
	
    HDC device_context = GetDC(window);

    int pixel_format = ChoosePixelFormat(device_context, &pfd); 
    SetPixelFormat(device_context, pixel_format, &pfd);

    HGLRC rendering_context = wglCreateContext(device_context);
	
	_device_context = device_context;
	_rendering_context = rendering_context;
}

void OpenGLContextWindows::flip()
{
	SwapBuffers(_device_context);
}

void OpenGLContextWindows::make_current_for_calling_thread()
{	
    wglMakeCurrent(_device_context, _rendering_context);
}

} // namespace bowtie
