#ifdef _WIN32
#pragma once

#include <Windows.h>
#include <foundation/keyboard.h>

namespace bowtie
{

class Window
{
public:
	typedef void (*WindowCreatedCallback)(HWND hwnd);
	typedef void (*WindowResizedCallback)(unsigned width, unsigned height);
	typedef void (*KeyDownCallback)(keyboard::Key key);
	typedef void (*KeyUpCallback)(keyboard::Key key);

	Window(HINSTANCE instance, WindowCreatedCallback window_created_callback, WindowResizedCallback window_resized_callback, KeyDownCallback key_down_callback, KeyUpCallback key_up_callback);
	void dispatch_messages();
	void invoke_window_created_callback(HWND hwnd);
	void invoke_window_resized_callback(unsigned width, unsigned height);
	void invoke_key_down_callback(keyboard::Key key);
	void invoke_key_up_callback(keyboard::Key key);
	
	void close();
	bool is_open() const { return _window_open; }

private:
	WindowCreatedCallback _window_created_callback;
	WindowResizedCallback _window_resized_callback;
	KeyDownCallback _key_down_callback;
	KeyUpCallback _key_up_callback;

	bool _window_open;
};

} // namespace bowtie

#endif