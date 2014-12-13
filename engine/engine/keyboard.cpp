#include "keyboard.h"
#include <foundation/hash.h>
#include <foundation/vector.h>
#include <foundation/string_utils.h>
#include <cassert>

namespace bowtie
{

namespace
{

struct NamedKey
{
	const char* name;
	platform::Key key;
};

static const NamedKey named_keys[] = 
{
	{ "F5", platform::Key::F5 },
	{ "Up", platform::Key::Up },
	{ "Down", platform::Key::Down },
	{ "Left", platform::Key::Left },
	{ "Right", platform::Key::Right },
	{ "A", platform::Key::A },
	{ "Z", platform::Key::Z },
	{ "R", platform::Key::R },
	{ "PageUp", platform::Key::PageUp },
	{ "PageDown", platform::Key::PageDown },
	{ "Tilde", platform::Key::Tilde }
};

}

namespace keyboard
{

platform::Key key_from_string(const char* key_str)
{
	for (unsigned i = 0; i < sizeof(named_keys) / sizeof(NamedKey); ++i)
	{
		if (strequal(key_str, named_keys[i].name))
			return named_keys[i].key;
	}

	assert(!"Key does not exist");
	return platform::Key::Up;
}

void reset_pressed_released(Keyboard* keyboard)
{
	memset(keyboard->keys_pressed, 0, sizeof(bool) * Keyboard::num_keys);
	memset(keyboard->keys_released, 0, sizeof(bool) * Keyboard::num_keys);
}

bool key_held(const Keyboard* keyboard, platform::Key key)
{
	return keyboard->keys_held[(unsigned)key];
}

bool key_pressed(const Keyboard* keyboard, platform::Key key)
{
	return keyboard->keys_pressed[(unsigned)key];
}

bool key_released(const Keyboard* keyboard, platform::Key key)
{
	return keyboard->keys_released[(unsigned)key];
}

void set_key_pressed(Keyboard* keyboard, platform::Key key)
{
	if (keyboard->keys_held[(unsigned)key])
        return;

    keyboard->keys_pressed[(unsigned)key] = true;
    keyboard->keys_held[(unsigned)key] = true;
}

void set_key_released(Keyboard* keyboard, platform::Key key)
{
	keyboard->keys_released[(unsigned)key] = true;
    keyboard->keys_held[(unsigned)key] = false;
}

} // namespace keyboard

} // namespace bowtie
