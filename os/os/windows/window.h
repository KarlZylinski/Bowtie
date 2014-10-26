#ifdef _WIN32
#pragma once

#include <Windows.h>
#include "keyboard.h"
#include <foundation/vector2u.h>

namespace bowtie
{

class Window
{
public:
	typedef void (*WindowCreatedCallback)(HWND hwnd, const Vector2u& resolution);
	typedef void (*WindowResizedCallback)(const Vector2u& resolution);
	typedef void (*KeyDownCallback)(keyboard::Key key);
	typedef void (*KeyUpCallback)(keyboard::Key key);

	Window(HINSTANCE instance, const Vector2u& resolution, WindowCreatedCallback window_created_callback, WindowResizedCallback window_resized_callback, KeyDownCallback key_down_callback, KeyUpCallback key_up_callback);
	void dispatch_messages();
	void invoke_window_created_callback(HWND hwnd, const Vector2u& resolution);
	void invoke_window_resized_callback(const Vector2u& resolution);
	void invoke_key_down_callback(keyboard::Key key);
	void invoke_key_up_callback(keyboard::Key key);
	const Vector2u& resolution() const;
	
	void close();
	bool is_open() const { return _window_open; }

private:
	WindowCreatedCallback _window_created_callback;
	WindowResizedCallback _window_resized_callback;
	KeyDownCallback _key_down_callback;
	KeyUpCallback _key_up_callback;

	bool _window_open;
	Vector2u _resolution;
};

} // namespace bowtie

#endif