#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main()
{
    auto display = XOpenDisplay(NULL);
    auto screen = DefaultScreen(display);
    auto depth = DefaultDepth(display, screen);
    XSetWindowAttributes attributes;
    attributes.border_pixel = BlackPixel(display,screen);
    attributes.background_pixel = WhitePixel(display,screen);
    attributes.override_redirect = True;

    auto window = XCreateWindow(
        display,
        RootWindow(display, screen),
        0,0,1280,720,
        2,depth,
        InputOutput,
        CopyFromParent,
        CWBackPixel|CWBorderPixel|CWOverrideRedirect,
        &attributes);

    XSizeHints size_hints;
    size_hints.flags = PPosition | PSize;
    size_hints.x = 0;
    size_hints.y = 0;
    size_hints.width = 1280;
    size_hints.height = 720;
    XSetNormalHints(display, window, &size_hints);
    XMapWindow(display, window);
    XFlush(display);
    while(true) {}
    return 0;
}

