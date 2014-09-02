#pragma once

#include <foundation/keyboard.h>
#include "render_resource_handle.h"

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