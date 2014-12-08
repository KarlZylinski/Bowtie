#ifdef _WIN32
#pragma once

#include <Windows.h>
#include "keyboard.h"
#include <foundation/vector2u.h>

namespace bowtie
{

namespace window
{
	typedef void(*WindowCreatedCallback)(HWND hwnd, const Vector2u& resolution);
	typedef void(*WindowResizedCallback)(const Vector2u& resolution);
	typedef void(*KeyDownCallback)(keyboard::Key key);
	typedef void(*KeyUpCallback)(keyboard::Key key);
}

struct Window
{
	window::WindowCreatedCallback _window_created_callback;
	window::WindowResizedCallback _window_resized_callback;
	window::KeyDownCallback _key_down_callback;
	window::KeyUpCallback _key_up_callback;
	bool is_open;
	Vector2u resolution;
	HWND hwnd;
};

namespace window
{
	void init(Window& w, HINSTANCE instance, const Vector2u& resolution, window::WindowCreatedCallback window_created_callback, window::WindowResizedCallback window_resized_callback,
		window::KeyDownCallback key_down_callback, window::KeyUpCallback key_up_callback);
	void dispatch_messages(Window& w);
	void invoke_window_created_callback(Window& w, HWND hwnd, const Vector2u& resolution);
	void invoke_window_resized_callback(Window& w, const Vector2u& resolution);
	void invoke_key_down_callback(Window& w, keyboard::Key key);
	void invoke_key_up_callback(Window& w, keyboard::Key key);
	void close(Window& w);
}

} // namespace bowtie

#endif