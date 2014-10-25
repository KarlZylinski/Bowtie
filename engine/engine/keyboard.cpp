#include "keyboard.h"
#include <foundation/hash.h>
#include <foundation/array.h>
#include <foundation/string_utils.h>
#include <cassert>

namespace bowtie
{

namespace
{

struct NamedKey
{
	const char* name;
	keyboard::Key key;
};

static const NamedKey named_keys[] = 
{
	{ "F5", keyboard::F5 },
	{ "Up", keyboard::Up },
	{ "Down", keyboard::Down },
	{ "Left", keyboard::Left },
	{ "Right", keyboard::Right },
	{ "A", keyboard::A },
	{ "Z", keyboard::Z },
	{ "R", keyboard::R },
	{ "PageUp", keyboard::PageUp },
	{ "PageDown", keyboard::PageDown },
	{ "Tilde", keyboard::Tilde }
};

}

namespace keyboard
{

keyboard::Key key_from_string(const char* key_str)
{
	for (unsigned i = 0; i < sizeof(named_keys) / sizeof(NamedKey); ++i)
	{
		if (strequal(key_str, named_keys[i].name))
			return named_keys[i].key;
	}

	assert(!"Key does not exist");
	return keyboard::Up;
}

void reset_pressed_released(Keyboard& keyboard)
{
	memset(keyboard.keys_pressed, 0, sizeof(bool) * Keyboard::num_keys);
	memset(keyboard.keys_released, 0, sizeof(bool) * Keyboard::num_keys);
}

bool key_held(const Keyboard& keyboard, keyboard::Key key)
{
	return keyboard.keys_held[key];
}

bool key_pressed(const Keyboard& keyboard, keyboard::Key key)
{
	return keyboard.keys_pressed[key];
}

bool key_released(const Keyboard& keyboard, keyboard::Key key)
{
	return keyboard.keys_released[key];
}

void set_key_pressed(Keyboard& keyboard, keyboard::Key key)
{
	if (keyboard.keys_held[key])
        return;

    keyboard.keys_pressed[key] = true;
    keyboard.keys_held[key] = true;
}

void set_key_released(Keyboard& keyboard, keyboard::Key key)
{
	keyboard.keys_released[key] = true;
    keyboard.keys_held[key] = false;
}

} // namespace keyboard

} // namespace bowtie
