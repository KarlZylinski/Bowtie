#ifdef _WIN32
#include "window.h"
#include <cassert>
#include <stdio.h>

namespace bowtie
{

namespace window
{

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void init(Window& w, HINSTANCE instance, const Vector2u& resolution, WindowCreatedCallback window_created_callback, WindowResizedCallback window_resized_callback,
		  KeyDownCallback key_down_callback, KeyUpCallback key_up_callback)
{
	w._key_down_callback = key_down_callback;
	w._key_up_callback = key_up_callback;
	w._window_resized_callback = window_resized_callback;
	w._window_created_callback = window_created_callback;
	w.resolution = resolution;
    WNDCLASS wc = {0}; 
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "Bowtie";
    wc.style = CS_OWNDC;
    assert(RegisterClass(&wc) && "Failed to register windows window class");
    w.is_open = true;
	int border_width = GetSystemMetrics(SM_CXFIXEDFRAME);
	int h_border_thickness = GetSystemMetrics(SM_CXSIZEFRAME) + border_width;
	int v_border_thickness = GetSystemMetrics(SM_CYSIZEFRAME) + border_width;
	int caption_thickness = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER);
	HWND hwnd = CreateWindow("Bowtie", "Bowtie", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, resolution.x + 2 * h_border_thickness, resolution.y + 2 * v_border_thickness + caption_thickness, 0, 0, instance, &w);
	w.hwnd = hwnd;
	SetWindowLong(hwnd, GWLP_USERDATA, (long)&w);
}

void dispatch_messages(Window&)
{
	MSG msg = {0};

	while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void invoke_window_created_callback(Window& w, HWND hwnd, const Vector2u& resolution)
{
	w._window_created_callback(hwnd, resolution);
}

void invoke_window_resized_callback(Window& w, const Vector2u& resolution)
{
	w._window_resized_callback(resolution);
}

void invoke_key_down_callback(Window& w, keyboard::Key key)
{
	w._key_down_callback(key);
}

void invoke_key_up_callback(Window& w, keyboard::Key key)
{
	w._key_up_callback(key);
}

void close(Window& w)
{
	w.is_open = false;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	Window* window = (Window*)GetWindowLong(hwnd,GWLP_USERDATA);

	switch(message)
    {
	case WM_CREATE:
		{
		Window* created_window = (Window*)(((LPCREATESTRUCT)lparam)->lpCreateParams);
		window::invoke_window_created_callback(*created_window, hwnd, created_window->resolution);
		}
		return 0;
	case WM_SIZE:
		{
		WORD width = LOWORD(lparam);
		WORD height = HIWORD(lparam);
		window->resolution = Vector2u(width, height);
		window::invoke_window_resized_callback(*window, window->resolution);
		}
		return 0;
	case WM_QUIT:
	case WM_CLOSE:
		window::close(*window);
		return 0;
	case WM_KEYDOWN:
		window::invoke_key_down_callback(*window, (keyboard::Key)(wparam));
		return 0;
	case WM_KEYUP:
		window::invoke_key_up_callback(*window, (keyboard::Key)(wparam));
		return 0;
    default:
		return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

} // namespace window

} // namespace bowtie

#endif