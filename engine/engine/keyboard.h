#pragma once

#include <foundation/keyboard.h>
#include <foundation/vector2.h>
#include <foundation/vector2u.h>
#include <foundation/matrix4.h>

#include "resource_handle.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

namespace bowtie
{

class Keyboard
{
public:
	static const unsigned num_keys = 120;

	Keyboard();
	static Keyboard from_previous_frame(const Keyboard& previous);
	static keyboard::Key key_from_string(const char* key_str);
	bool key_held(keyboard::Key key) const;
	bool key_pressed(keyboard::Key key) const;
	bool key_released(keyboard::Key key) const;
	void set_key_pressed(keyboard::Key key);
	void set_key_released(keyboard::Key key);

private:
	bool _keys_held[num_keys];
	bool _keys_pressed[num_keys];
	bool _keys_released[num_keys];
};

}