#include "window.h"
#include <cassert>
#include <stdio.h>

namespace bowtie
{

namespace windows
{

namespace internal
{

Key key_from_windows_key_code(WPARAM key, LPARAM flags)
{
    switch (key)
    {
        case VK_SHIFT:
        {
            auto left_shift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
            auto shift_check_code = (flags & (0xFF << 16)) >> 16;
            return shift_check_code == left_shift ? Key::LShift : Key::RShift;
        }

        case VK_MENU:       return (HIWORD(flags) & KF_EXTENDED) ? Key::RAlt : Key::LAlt;
        case VK_CONTROL:    return (HIWORD(flags) & KF_EXTENDED) ? Key::RControl : Key::LControl;
        case VK_LWIN:       return Key::LSystem;
        case VK_RWIN:       return Key::RSystem;
        case VK_APPS:       return Key::Menu;
        case VK_OEM_1:      return Key::SemiColon;
        case VK_OEM_2:      return Key::Slash;
        case VK_OEM_PLUS:   return Key::Equal;
        case VK_OEM_MINUS:  return Key::Dash;
        case VK_OEM_4:      return Key::LBracket;
        case VK_OEM_6:      return Key::RBracket;
        case VK_OEM_COMMA:  return Key::Comma;
        case VK_OEM_PERIOD: return Key::Period;
        case VK_OEM_7:      return Key::Quote;
        case VK_OEM_5:      return Key::BackSlash;
        case VK_OEM_3:      return Key::Tilde;
        case VK_ESCAPE:     return Key::Escape;
        case VK_SPACE:      return Key::Space;
        case VK_RETURN:     return Key::Return;
        case VK_BACK:       return Key::BackSpace;
        case VK_TAB:        return Key::Tab;
        case VK_PRIOR:      return Key::PageUp;
        case VK_NEXT:       return Key::PageDown;
        case VK_END:        return Key::End;
        case VK_HOME:       return Key::Home;
        case VK_INSERT:     return Key::Insert;
        case VK_DELETE:     return Key::Delete;
        case VK_ADD:        return Key::Add;
        case VK_SUBTRACT:   return Key::Subtract;
        case VK_MULTIPLY:   return Key::Multiply;
        case VK_DIVIDE:     return Key::Divide;
        case VK_PAUSE:      return Key::Pause;
        case VK_F1:         return Key::F1;
        case VK_F2:         return Key::F2;
        case VK_F3:         return Key::F3;
        case VK_F4:         return Key::F4;
        case VK_F5:         return Key::F5;
        case VK_F6:         return Key::F6;
        case VK_F7:         return Key::F7;
        case VK_F8:         return Key::F8;
        case VK_F9:         return Key::F9;
        case VK_F10:        return Key::F10;
        case VK_F11:        return Key::F11;
        case VK_F12:        return Key::F12;
        case VK_F13:        return Key::F13;
        case VK_F14:        return Key::F14;
        case VK_F15:        return Key::F15;
        case VK_LEFT:       return Key::Left;
        case VK_RIGHT:      return Key::Right;
        case VK_UP:         return Key::Up;
        case VK_DOWN:       return Key::Down;
        case VK_NUMPAD0:    return Key::Numpad0;
        case VK_NUMPAD1:    return Key::Numpad1;
        case VK_NUMPAD2:    return Key::Numpad2;
        case VK_NUMPAD3:    return Key::Numpad3;
        case VK_NUMPAD4:    return Key::Numpad4;
        case VK_NUMPAD5:    return Key::Numpad5;
        case VK_NUMPAD6:    return Key::Numpad6;
        case VK_NUMPAD7:    return Key::Numpad7;
        case VK_NUMPAD8:    return Key::Numpad8;
        case VK_NUMPAD9:    return Key::Numpad9;
        case 'A':           return Key::A;
        case 'Z':           return Key::Z;
        case 'E':           return Key::E;
        case 'R':           return Key::R;
        case 'T':           return Key::T;
        case 'Y':           return Key::Y;
        case 'U':           return Key::U;
        case 'I':           return Key::I;
        case 'O':           return Key::O;
        case 'P':           return Key::P;
        case 'Q':           return Key::Q;
        case 'S':           return Key::S;
        case 'D':           return Key::D;
        case 'F':           return Key::F;
        case 'G':           return Key::G;
        case 'H':           return Key::H;
        case 'J':           return Key::J;
        case 'K':           return Key::K;
        case 'L':           return Key::L;
        case 'M':           return Key::M;
        case 'W':           return Key::W;
        case 'X':           return Key::X;
        case 'C':           return Key::C;
        case 'V':           return Key::V;
        case 'B':           return Key::B;
        case 'N':           return Key::N;
        case '0':           return Key::Num0;
        case '1':           return Key::Num1;
        case '2':           return Key::Num2;
        case '3':           return Key::Num3;
        case '4':           return Key::Num4;
        case '5':           return Key::Num5;
        case '6':           return Key::Num6;
        case '7':           return Key::Num7;
        case '8':           return Key::Num8;
        case '9':           return Key::Num9;
    }

    return Key::Unknown;
}

}

namespace window
{

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void init(Window* w, HINSTANCE instance, const Vector2u* resolution, WindowCreatedCallback window_created_callback, WindowResizedCallback window_resized_callback,
          KeyDownCallback key_down_callback, KeyUpCallback key_up_callback)
{
    w->_key_down_callback = key_down_callback;
    w->_key_up_callback = key_up_callback;
    w->_window_resized_callback = window_resized_callback;
    w->_window_created_callback = window_created_callback;
    w->resolution = *resolution;
    WNDCLASS wc = {}; 
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"Bowtie";
    wc.style = CS_OWNDC;
    assert(RegisterClass(&wc) && "Failed to register windows window class");
    w->is_open = true;
    int32 border_width = GetSystemMetrics(SM_CXFIXEDFRAME);
    int32 h_border_thickness = GetSystemMetrics(SM_CXSIZEFRAME) + border_width;
    int32 v_border_thickness = GetSystemMetrics(SM_CYSIZEFRAME) + border_width;
    int32 caption_thickness = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER);
    HWND hwnd = CreateWindow(L"Bowtie", L"Bowtie", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, resolution->x + 2 * h_border_thickness, resolution->y + 2 * v_border_thickness + caption_thickness, 0, 0, instance, w);
    w->hwnd = hwnd;
    SetWindowLong(hwnd, GWLP_USERDATA, (long)w);
}

void dispatch_messages(Window*)
{
    MSG msg = {};

    while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void invoke_window_created_callback(Window* w, HWND hwnd, const Vector2u* resolution)
{
    w->_window_created_callback(hwnd, resolution);
}

void invoke_window_resized_callback(Window* w, const Vector2u* resolution)
{
    w->_window_resized_callback(resolution);
}

void invoke_key_down_callback(Window* w, Key key)
{
    w->_key_down_callback(key);
}

void invoke_key_up_callback(Window* w, Key key)
{
    w->_key_up_callback(key);
}

void close(Window* w)
{
    w->is_open = false;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    Window* window = (Window*)GetWindowLong(hwnd,GWLP_USERDATA);

    switch(message)
    {
    case WM_CREATE:
        {
        Window* created_window = (Window*)(((LPCREATESTRUCT)lparam)->lpCreateParams);
        invoke_window_created_callback(created_window, hwnd, &created_window->resolution);
        }
        return 0;
    case WM_SIZE:
        {
        WORD width = LOWORD(lparam);
        WORD height = HIWORD(lparam);
        window->resolution = vector2u::create(width, height);
        invoke_window_resized_callback(window, &window->resolution);
        }
        return 0;
    case WM_QUIT:
    case WM_CLOSE:
        close(window);
        return 0;
    case WM_KEYDOWN:
        invoke_key_down_callback(window, internal::key_from_windows_key_code(wparam, lparam));
        return 0;
    case WM_KEYUP:
        invoke_key_up_callback(window, internal::key_from_windows_key_code(wparam, lparam));
        return 0;
    default:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }
}

} // namespace window

} // namespace windows

} // namespace bowtie
