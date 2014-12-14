#include "opengl_context.h"

namespace bowtie
{

namespace internal
{

void enable_vsync()
{
    typedef bool (APIENTRY *PFNWGLSWAPINTERVALEXTPROC) (int32 interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
    wglSwapIntervalEXT(-1); 
}

void flip(PlatformRendererContextData* context)
{
    SwapBuffers(context->device_context);
}

void make_current_for_calling_thread(PlatformRendererContextData* context)
{    
    wglMakeCurrent(context->device_context, context->rendering_context);
    enable_vsync();
}

}

namespace windows
{

namespace opengl_context
{

void init(PlatformRendererContextData* context, HWND hwnd)
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
    
    HDC device_context = GetDC(hwnd);
    int32 pixel_format = ChoosePixelFormat(device_context, &pfd); 
    SetPixelFormat(device_context, pixel_format, &pfd);
    HGLRC rendering_context = wglCreateContext(device_context);
    context->device_context = device_context;
    context->rendering_context = rendering_context;
}

RendererContext create()
{
    RendererContext c;
    c.flip = internal::flip;
    c.make_current_for_calling_thread = internal::make_current_for_calling_thread;
    return c;
}

}

}

} // namespace bowtie
