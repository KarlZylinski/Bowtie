#include "keyboard.h"
#include <foundation/hash.h>
#include <foundation/array.h>
#include <foundation/string_utils.h>
#include <cassert>

namespace bowtie
{

Keyboard::Keyboard()
{
	memset(_keys_held, 0, sizeof(_keys_held));
	memset(_keys_pressed, 0, sizeof(_keys_pressed));
	memset(_keys_released, 0, sizeof(_keys_released));
}

Keyboard Keyboard::from_previous_frame(const Keyboard& previous)
{
	auto keyboard = Keyboard();
	memcpy(keyboard._keys_held, previous._keys_held, sizeof(keyboard._keys_held));
	return keyboard;
}

struct NamedKey
{
	const char* name;
	keyboard::Key key;
};

static const NamedKey named_keys[] = 
{
	{ "Up", keyboard::Up },
	{ "Down", keyboard::Down },
	{ "Left", keyboard::Left },
	{ "Right", keyboard::Right },
	{ "A", keyboard::A },
	{ "Z", keyboard::Z },
	{ "PageUp", keyboard::PageUp },
	{ "PageDown", keyboard::PageDown }
};

keyboard::Key Keyboard::key_from_string(const char* key_str)
{
	for (unsigned i = 0; i < sizeof(named_keys) / sizeof(NamedKey); ++i)
	{
		if (strequal(key_str, named_keys[i].name))
			return named_keys[i].key;
	}

	assert(!"Key does not exist");
	return keyboard::Up;
}

bool Keyboard::key_held(keyboard::Key key) const
{
	return _keys_held[key];
}

bool Keyboard::key_pressed(keyboard::Key key) const
{
	return _keys_pressed[key];
}

bool Keyboard::key_released(keyboard::Key key) const
{
	return _keys_released[key];
}

void Keyboard::set_key_pressed(keyboard::Key key)
{
	if (_keys_held[key] == true)
        return;

    _keys_pressed[key] = true;
    _keys_held[key] = true;
}

void Keyboard::set_key_released(keyboard::Key key)
{
	_keys_released[key] = true;
    _keys_held[key] = false;
}

} // namespace bowtie
