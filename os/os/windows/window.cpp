#ifdef _WIN32
#include "window.h"
#include <cassert>
#include <stdio.h>

#include <foundation/keyboard.h>

namespace bowtie
{

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

Window::Window(HINSTANCE instance, const Vector2u& resolution, WindowCreatedCallback window_created_callback, WindowResizedCallback window_resized_callback, KeyDownCallback key_down_callback, KeyUpCallback key_up_callback) :
	_window_created_callback(window_created_callback),
	_window_resized_callback(window_resized_callback),
	_key_down_callback(key_down_callback),
	_key_up_callback(key_up_callback),
	_resolution(resolution)
{
    WNDCLASS wc      = {0}; 
    wc.lpfnWndProc   = window_proc;
    wc.hInstance     = instance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "Bowtie";
    wc.style = CS_OWNDC;
    assert(RegisterClass(&wc) && "Failed to register windows window class");
    _window_open = true;
	HWND hwnd = CreateWindow("Bowtie", "Bowtie", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, resolution.x, resolution.y, 0, 0, instance, this);
	SetWindowLong(hwnd, GWLP_USERDATA, (long)this);
}

void Window::dispatch_messages()
{
	MSG msg = {0};

	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::invoke_window_created_callback(HWND hwnd, const Vector2u& resolution)
{
	_window_created_callback(hwnd, resolution);
}

void Window::invoke_window_resized_callback(const Vector2u& resolution)
{
	_window_resized_callback(resolution);
}

void Window::invoke_key_down_callback(keyboard::Key key)
{
	_key_down_callback(key);
}

void Window::invoke_key_up_callback(keyboard::Key key)
{
	_key_up_callback(key);
}

void Window::close()
{
	_window_open = false;
}

const Vector2u& Window::resolution() const
{
	return _resolution;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	Window* window = (Window*)GetWindowLong(hwnd,GWLP_USERDATA);

	switch(message)
    {
	case WM_CREATE:
		{
		Window* created_window = (Window*)(((LPCREATESTRUCT)lparam)->lpCreateParams);
		created_window->invoke_window_created_callback(hwnd, window->resolution());
		}
		return 0;
	case WM_SIZE:
		{
		WORD width = LOWORD(lparam);
		WORD height = HIWORD(lparam);
		window->invoke_window_resized_callback(Vector2u(width, height));
		}
		return 0;
	case WM_QUIT:
	case WM_CLOSE:
		window->close();
		return 0;
	case WM_KEYDOWN:
		window->invoke_key_down_callback((keyboard::Key)(wparam));
		return 0;
	case WM_KEYUP:
		window->invoke_key_up_callback((keyboard::Key)(wparam));
		return 0;
    default:
		return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

} // namespace bowtie

#endif