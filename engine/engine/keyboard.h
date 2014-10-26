#pragma once

#ifdef _WIN32
	#include <os/windows/keyboard.h>
#endif

#include "render_resource_handle.h"

namespace bowtie
{

struct Keyboard
{
	static const unsigned num_keys = 120;
	bool keys_held[num_keys];
	bool keys_pressed[num_keys];
	bool keys_released[num_keys];
};

namespace keyboard
{
	keyboard::Key key_from_string(const char* key_str);
	void reset_pressed_released(Keyboard& keyboard);
	bool key_held(const Keyboard& keyboard, keyboard::Key key);
	bool key_pressed(const Keyboard& keyboard, keyboard::Key key);
	bool key_released(const Keyboard& keyboard, keyboard::Key key);
	void set_key_pressed(Keyboard& keyboard, keyboard::Key key);
	void set_key_released(Keyboard& keyboard, keyboard::Key key);
}

}