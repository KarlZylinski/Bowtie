#pragma once

#include "renderer/render_resource_handle.h"
#include <base/key.h>

namespace bowtie
{

struct Keyboard
{
    static const uint32 num_keys = 120;
    bool keys_held[num_keys];
    bool keys_pressed[num_keys];
    bool keys_released[num_keys];
};

namespace keyboard
{
    Key key_from_string(const char* key_str);
    void reset_pressed_released(Keyboard* keyboard);
    bool key_held(const Keyboard* keyboard, Key key);
    bool key_pressed(const Keyboard* keyboard, Key key);
    bool key_released(const Keyboard* keyboard, Key key);
    void set_key_pressed(Keyboard* keyboard, Key key);
    void set_key_released(Keyboard* keyboard, Key key);
}

}