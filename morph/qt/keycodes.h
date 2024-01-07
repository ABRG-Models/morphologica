#pragma once

#include <morph/keys.h>
#include <QKeyEvent>

namespace morph {
    namespace qt {

        // Pass in a Qt key code and receive back the morph (i.e. GLFW) key code.
        constexpr int qtkey_to_morphkey (int qt_keycode)
        {
            int mkc = morph::key::unknown;
            switch (qt_keycode) {

            case Qt::Key_Space:
                mkc = morph::key::space;
                break;
            case Qt::Key_Apostrophe:
                mkc = morph::key::apostrophe;
                break;
            case Qt::Key_Comma:
                mkc = morph::key::comma;
                break;
            case Qt::Key_Minus:
                mkc = morph::key::minus;
                break;
            case Qt::Key_Period:
                mkc = morph::key::period;
                break;
            case Qt::Key_Slash:
                mkc = morph::key::slash;
                break;

            case Qt::Key_0:
                mkc = morph::key::n0;
                break;
            case Qt::Key_1:
                mkc = morph::key::n1;
                break;
            case Qt::Key_2:
                mkc = morph::key::n2;
                break;
            case Qt::Key_3:
                mkc = morph::key::n3;
                break;
            case Qt::Key_4:
                mkc = morph::key::n4;
                break;
            case Qt::Key_5:
                mkc = morph::key::n5;
                break;
            case Qt::Key_6:
                mkc = morph::key::n6;
                break;
            case Qt::Key_7:
                mkc = morph::key::n7;
                break;
            case Qt::Key_8:
                mkc = morph::key::n8;
                break;
            case Qt::Key_9:
                mkc = morph::key::n9;
                break;

            case Qt::Key_Semicolon:
                mkc = morph::key::semicolon;
                break;
            case Qt::Key_Equal:
                mkc = morph::key::equal;
                break;

            case Qt::Key_A:
                mkc = morph::key::a;
                break;
            case Qt::Key_B:
                mkc = morph::key::b;
                break;
            case Qt::Key_C:
                mkc = morph::key::c;
                break;
            case Qt::Key_D:
                mkc = morph::key::d;
                break;
            case Qt::Key_E:
                mkc = morph::key::e;
                break;
            case Qt::Key_F:
                mkc = morph::key::f;
                break;
            case Qt::Key_G:
                mkc = morph::key::g;
                break;
            case Qt::Key_H:
                mkc = morph::key::h;
                break;
            case Qt::Key_I:
                mkc = morph::key::i;
                break;
            case Qt::Key_J:
                mkc = morph::key::j;
                break;
            case Qt::Key_K:
                mkc = morph::key::k;
                break;
            case Qt::Key_L:
                mkc = morph::key::l;
                break;
            case Qt::Key_M:
                mkc = morph::key::m;
                break;
            case Qt::Key_N:
                mkc = morph::key::n;
                break;
            case Qt::Key_O:
                mkc = morph::key::o;
                break;
            case Qt::Key_P:
                mkc = morph::key::p;
                break;
            case Qt::Key_Q:
                mkc = morph::key::q;
                break;
            case Qt::Key_R:
                mkc = morph::key::r;
                break;
            case Qt::Key_S:
                mkc = morph::key::s;
                break;
            case Qt::Key_T:
                mkc = morph::key::t;
                break;
            case Qt::Key_U:
                mkc = morph::key::u;
                break;
            case Qt::Key_V:
                mkc = morph::key::v;
                break;
            case Qt::Key_W:
                mkc = morph::key::w;
                break;
            case Qt::Key_X:
                mkc = morph::key::x;
                break;
            case Qt::Key_Y:
                mkc = morph::key::y;
                break;
            case Qt::Key_Z:
                mkc = morph::key::z;
                break;

            case Qt::Key_BracketLeft:
                mkc = morph::key::left_bracket;
                break;
            case Qt::Key_Backslash:
                mkc = morph::key::backslash;
                break;
            case Qt::Key_BracketRight:
                mkc = morph::key::right_bracket;
                break;
            case Qt::Key_QuoteLeft:
                mkc = morph::key::grave_accent;
                break;
            case Qt::Key_exclamdown: // guess
                mkc = morph::key::world_1;
                break;
            case Qt::Key_cent:       // guess
                mkc = morph::key::world_2;
                break;

            case Qt::Key_Escape:
                mkc = morph::key::escape;
                break;
            case Qt::Key_Enter:
                mkc = morph::key::enter;
                break;
            case Qt::Key_Tab:
                mkc = morph::key::tab;
                break;
            case Qt::Key_Backspace:
                mkc = morph::key::backspace;
                break;
            case Qt::Key_Insert:
                mkc = morph::key::insert;
                break;
            case Qt::Key_Delete:
                mkc = morph::key::delete;
                break;
            case Qt::Key_Right:
                mkc = morph::key::right;
                break;
            case Qt::Key_Left:
                mkc = morph::key::left;
                break;
            case Qt::Key_Down:
                mkc = morph::key::down;
                break;
            case Qt::Key_Up:
                mkc = morph::key::up;
                break;
            case Qt::Key_PageUp:
                mkc = morph::key::page_up;
                break;
            case Qt::Key_PageDown:
                mkc = morph::key::page_down;
                break;
            case Qt::Key_Home:
                mkc = morph::key::home;
                break;
            case Qt::Key_End:
                mkc = morph::key::end;
                break;
            case Qt::Key_CapsLock:
                mkc = morph::key::caps_lock;
                break;
            case Qt::Key_ScrollLock:
                mkc = morph::key::scroll_lock;
                break;
            case Qt::Key_NumLock:
                mkc = morph::key::num_lock;
                break;
            case Qt::Key_Print:
                mkc = morph::key::print_screen;
                break;
            case Qt::Key_Pause:
                mkc = morph::key::pause;
                break;

            case Qt::Key_F1:
                mkc = morph::key::f1;
                break;
            case Qt::Key_F2:
                mkc = morph::key::f2;
                break;
            case Qt::Key_F3:
                mkc = morph::key::f3;
                break;
            case Qt::Key_F4:
                mkc = morph::key::f4;
                break;
            case Qt::Key_F5:
                mkc = morph::key::f5;
                break;
            case Qt::Key_F6:
                mkc = morph::key::f6;
                break;
            case Qt::Key_F7:
                mkc = morph::key::f7;
                break;
            case Qt::Key_F8:
                mkc = morph::key::f8;
                break;
            case Qt::Key_F9:
                mkc = morph::key::f9;
                break;
            case Qt::Key_F10:
                mkc = morph::key::f10;
                break;
            case Qt::Key_F11:
                mkc = morph::key::f11;
                break;
            case Qt::Key_F12:
                mkc = morph::key::f12;
                break;
            case Qt::Key_F13:
                mkc = morph::key::f13;
                break;
            case Qt::Key_F14:
                mkc = morph::key::f14;
                break;
            case Qt::Key_F15:
                mkc = morph::key::f15;
                break;
            case Qt::Key_F16:
                mkc = morph::key::f16;
                break;
            case Qt::Key_F17:
                mkc = morph::key::f17;
                break;
            case Qt::Key_F18:
                mkc = morph::key::f18;
                break;
            case Qt::Key_F19:
                mkc = morph::key::f19;
                break;
            case Qt::Key_F20:
                mkc = morph::key::f20;
                break;
            case Qt::Key_F21:
                mkc = morph::key::f21;
                break;
            case Qt::Key_F22:
                mkc = morph::key::f22;
                break;
            case Qt::Key_F23:
                mkc = morph::key::f23;
                break;
            case Qt::Key_F24:
                mkc = morph::key::f24;
                break;
            case Qt::Key_F25:
                mkc = morph::key::f25;
                break;

            case Qt::Key_division:
                mkc = morph::key::kp_divide;
                break;
            case Qt::Key_multiply:
                mkc = morph::key::kp_multiply;
                break;

            case Qt::Key_Shift:
                mkc = morph::key::left_shift;
                break;
            case Qt::Key_Control:
                mkc = morph::key::left_control;
                break;
            case Qt::Key_Alt:
                mkc = morph::key::left_alt;
                break;
            case Qt::Key_AltGr:
                mkc = morph::key::right_alt;
                break;
            case Qt::Key_Super_L:
                mkc = morph::key::left_super;
                break;
            case Qt::Key_Super_R:
                mkc = morph::key::right_super;
                break;

            default:
                break;
            }
            return mkc;
        }
    }
}
