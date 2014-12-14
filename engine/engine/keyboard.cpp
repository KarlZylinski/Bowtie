#include "keyboard.h"
#include <base/hash.h>
#include <base/vector.h>
#include <base/string_utils.h>
#include <cassert>

namespace bowtie
{

namespace
{

struct NamedKey
{
    const char* name;
    Key key;
};

static const NamedKey named_keys[] = 
{
    {"A", Key::A},
    {"B", Key::B},
    {"C", Key::C},
    {"D", Key::D},
    {"E", Key::E},
    {"F", Key::F},
    {"G", Key::G},
    {"H", Key::H},
    {"I", Key::I},
    {"J", Key::J},
    {"K", Key::K},
    {"L", Key::L},
    {"M", Key::M},
    {"N", Key::N},
    {"O", Key::O},
    {"P", Key::P},
    {"Q", Key::Q},
    {"R", Key::R},
    {"S", Key::S},
    {"T", Key::T},
    {"U", Key::U},
    {"V", Key::V},
    {"W", Key::W},
    {"X", Key::X},
    {"Y", Key::Y},
    {"Z", Key::Z},
    {"Num0", Key::Num0},
    {"Num1", Key::Num1},
    {"Num2", Key::Num2},
    {"Num3", Key::Num3},
    {"Num4", Key::Num4},
    {"Num5", Key::Num5},
    {"Num6", Key::Num6},
    {"Num7", Key::Num7},
    {"Num8", Key::Num8},
    {"Num9", Key::Num9},
    {"Escape", Key::Escape},
    {"LControl", Key::LControl},
    {"LShift", Key::LShift},
    {"LAlt", Key::LAlt},
    {"LSystem", Key::LSystem},
    {"RControl", Key::RControl},
    {"RShift", Key::RShift},
    {"RAlt", Key::RAlt},
    {"RSystem", Key::RSystem},
    {"Menu", Key::Menu},
    {"LBracket", Key::LBracket},
    {"RBracket", Key::RBracket},
    {"SemiColon", Key::SemiColon},
    {"Comma", Key::Comma},
    {"Period", Key::Period},
    {"Quote", Key::Quote},
    {"Slash", Key::Slash},
    {"BackSlash", Key::BackSlash},
    {"Tilde", Key::Tilde},
    {"Equal", Key::Equal},
    {"Dash", Key::Dash},
    {"Space", Key::Space},
    {"Return", Key::Return},
    {"BackSpace", Key::BackSpace},
    {"Tab", Key::Tab},
    {"PageUp", Key::PageUp},
    {"PageDown", Key::PageDown},
    {"End", Key::End},
    {"Home", Key::Home},
    {"Insert", Key::Insert},
    {"Delete", Key::Delete},
    {"Add", Key::Add},
    {"Subtract", Key::Subtract},
    {"Multiply", Key::Multiply},
    {"Divide", Key::Divide},
    {"Left", Key::Left},
    {"Right", Key::Right},
    {"Up", Key::Up},
    {"Down", Key::Down},
    {"Numpad0", Key::Numpad0},
    {"Numpad1", Key::Numpad1},
    {"Numpad2", Key::Numpad2},
    {"Numpad3", Key::Numpad3},
    {"Numpad4", Key::Numpad4},
    {"Numpad5", Key::Numpad5},
    {"Numpad6", Key::Numpad6},
    {"Numpad7", Key::Numpad7},
    {"Numpad8", Key::Numpad8},
    {"Numpad9", Key::Numpad9},
    {"F1", Key::F1},
    {"F2", Key::F2},
    {"F3", Key::F3},
    {"F4", Key::F4},
    {"F5", Key::F5},
    {"F6", Key::F6},
    {"F7", Key::F7},
    {"F8", Key::F8},
    {"F9", Key::F9},
    {"F10", Key::F10},
    {"F11", Key::F11},
    {"F12", Key::F12},
    {"F13", Key::F13},
    {"F14", Key::F14},
    {"F15", Key::F15},
    {"Pause", Key::Pause},
    {"NumKeys", Key::NumKeys}
};

}

namespace keyboard
{

Key key_from_string(const char* key_str)
{
    for (uint32 i = 0; i < sizeof(named_keys) / sizeof(NamedKey); ++i)
    {
        if (strequal(key_str, named_keys[i].name))
            return named_keys[i].key;
    }

    return Key::Unknown;
}

void reset_pressed_released(Keyboard* keyboard)
{
    memset(keyboard->keys_pressed, 0, sizeof(bool) * Keyboard::num_keys);
    memset(keyboard->keys_released, 0, sizeof(bool) * Keyboard::num_keys);
}

bool key_held(const Keyboard* keyboard, Key key)
{
    return keyboard->keys_held[(uint32)key];
}

bool key_pressed(const Keyboard* keyboard, Key key)
{
    return keyboard->keys_pressed[(uint32)key];
}

bool key_released(const Keyboard* keyboard, Key key)
{
    return keyboard->keys_released[(uint32)key];
}

void set_key_pressed(Keyboard* keyboard, Key key)
{
    if (keyboard->keys_held[(uint32)key])
        return;

    keyboard->keys_pressed[(uint32)key] = true;
    keyboard->keys_held[(uint32)key] = true;
}

void set_key_released(Keyboard* keyboard, Key key)
{
    keyboard->keys_released[(uint32)key] = true;
    keyboard->keys_held[(uint32)key] = false;
}

} // namespace keyboard

} // namespace bowtie
