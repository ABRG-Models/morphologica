#pragma once

#include <morph/keys.h>
#include <QKeyEvent>

namespace morph {
    namespace qt {

        // Pass in a Qt key code and receive back the morph (i.e. GLFW) key code.
        constexpr int qtkey_to_morphkey (int qt_keycode)
        {
            int mkc = morph::key::UNKNOWN;
            switch (qt_keycode) {

            case Qt::Key_Space:
                mkc = morph::key::SPACE;
                break;
            case Qt::Key_Apostrophe:
                mkc = morph::key::APOSTROPHE;
                break;
            case Qt::Key_Comma:
                mkc = morph::key::COMMA;
                break;
            case Qt::Key_Minus:
                mkc = morph::key::MINUS;
                break;
            case Qt::Key_Period:
                mkc = morph::key::PERIOD;
                break;
            case Qt::Key_Slash:
                mkc = morph::key::SLASH;
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
                mkc = morph::key::SEMICOLON;
                break;
            case Qt::Key_Equal:
                mkc = morph::key::EQUAL;
                break;

            case Qt::Key_A:
                mkc = morph::key::A;
                break;
            case Qt::Key_B:
                mkc = morph::key::B;
                break;
            case Qt::Key_C:
                mkc = morph::key::C;
                break;
            case Qt::Key_D:
                mkc = morph::key::D;
                break;
            case Qt::Key_E:
                mkc = morph::key::E;
                break;
            case Qt::Key_F:
                mkc = morph::key::F;
                break;
            case Qt::Key_G:
                mkc = morph::key::G;
                break;
            case Qt::Key_H:
                mkc = morph::key::H;
                break;
            case Qt::Key_I:
                mkc = morph::key::I;
                break;
            case Qt::Key_J:
                mkc = morph::key::J;
                break;
            case Qt::Key_K:
                mkc = morph::key::K;
                break;
            case Qt::Key_L:
                mkc = morph::key::L;
                break;
            case Qt::Key_M:
                mkc = morph::key::M;
                break;
            case Qt::Key_N:
                mkc = morph::key::N;
                break;
            case Qt::Key_O:
                mkc = morph::key::O;
                break;
            case Qt::Key_P:
                mkc = morph::key::P;
                break;
            case Qt::Key_Q:
                mkc = morph::key::Q;
                break;
            case Qt::Key_R:
                mkc = morph::key::R;
                break;
            case Qt::Key_S:
                mkc = morph::key::S;
                break;
            case Qt::Key_T:
                mkc = morph::key::T;
                break;
            case Qt::Key_U:
                mkc = morph::key::U;
                break;
            case Qt::Key_V:
                mkc = morph::key::V;
                break;
            case Qt::Key_W:
                mkc = morph::key::W;
                break;
            case Qt::Key_X:
                mkc = morph::key::X;
                break;
            case Qt::Key_Y:
                mkc = morph::key::Y;
                break;
            case Qt::Key_Z:
                mkc = morph::key::Z;
                break;

            case Qt::Key_BracketLeft:
                mkc = morph::key::LEFT_BRACKET;
                break;
            case Qt::Key_Backslash:
                mkc = morph::key::BACKSLASH;
                break;
            case Qt::Key_BracketRight:
                mkc = morph::key::RIGHT_BRACKET;
                break;
            case Qt::Key_QuoteLeft:
                mkc = morph::key::GRAVE_ACCENT;
                break;
            case Qt::Key_exclamdown: // guess
                mkc = morph::key::WORLD_1;
                break;
            case Qt::Key_cent:       // guess
                mkc = morph::key::WORLD_2;
                break;

            case Qt::Key_Escape:
                mkc = morph::key::ESCAPE;
                break;
            case Qt::Key_Enter:
                mkc = morph::key::ENTER;
                break;
            case Qt::Key_Tab:
                mkc = morph::key::TAB;
                break;
            case Qt::Key_Backspace:
                mkc = morph::key::BACKSPACE;
                break;
            case Qt::Key_Insert:
                mkc = morph::key::INSERT;
                break;
            case Qt::Key_Delete:
                mkc = morph::key::DELETE;
                break;
            case Qt::Key_Right:
                mkc = morph::key::RIGHT;
                break;
            case Qt::Key_Left:
                mkc = morph::key::LEFT;
                break;
            case Qt::Key_Down:
                mkc = morph::key::DOWN;
                break;
            case Qt::Key_Up:
                mkc = morph::key::UP;
                break;
            case Qt::Key_PageUp:
                mkc = morph::key::PAGE_UP;
                break;
            case Qt::Key_PageDown:
                mkc = morph::key::PAGE_DOWN;
                break;
            case Qt::Key_Home:
                mkc = morph::key::HOME;
                break;
            case Qt::Key_End:
                mkc = morph::key::END;
                break;
            case Qt::Key_CapsLock:
                mkc = morph::key::CAPS_LOCK;
                break;
            case Qt::Key_ScrollLock:
                mkc = morph::key::SCROLL_LOCK;
                break;
            case Qt::Key_NumLock:
                mkc = morph::key::NUM_LOCK;
                break;
            case Qt::Key_Print:
                mkc = morph::key::PRINT_SCREEN;
                break;
            case Qt::Key_Pause:
                mkc = morph::key::PAUSE;
                break;

            case Qt::Key_F1:
                mkc = morph::key::F1;
                break;
            case Qt::Key_F2:
                mkc = morph::key::F2;
                break;
            case Qt::Key_F3:
                mkc = morph::key::F3;
                break;
            case Qt::Key_F4:
                mkc = morph::key::F4;
                break;
            case Qt::Key_F5:
                mkc = morph::key::F5;
                break;
            case Qt::Key_F6:
                mkc = morph::key::F6;
                break;
            case Qt::Key_F7:
                mkc = morph::key::F7;
                break;
            case Qt::Key_F8:
                mkc = morph::key::F8;
                break;
            case Qt::Key_F9:
                mkc = morph::key::F9;
                break;
            case Qt::Key_F10:
                mkc = morph::key::F10;
                break;
            case Qt::Key_F11:
                mkc = morph::key::F11;
                break;
            case Qt::Key_F12:
                mkc = morph::key::F12;
                break;
            case Qt::Key_F13:
                mkc = morph::key::F13;
                break;
            case Qt::Key_F14:
                mkc = morph::key::F14;
                break;
            case Qt::Key_F15:
                mkc = morph::key::F15;
                break;
            case Qt::Key_F16:
                mkc = morph::key::F16;
                break;
            case Qt::Key_F17:
                mkc = morph::key::F17;
                break;
            case Qt::Key_F18:
                mkc = morph::key::F18;
                break;
            case Qt::Key_F19:
                mkc = morph::key::F19;
                break;
            case Qt::Key_F20:
                mkc = morph::key::F20;
                break;
            case Qt::Key_F21:
                mkc = morph::key::F21;
                break;
            case Qt::Key_F22:
                mkc = morph::key::F22;
                break;
            case Qt::Key_F23:
                mkc = morph::key::F23;
                break;
            case Qt::Key_F24:
                mkc = morph::key::F24;
                break;
            case Qt::Key_F25:
                mkc = morph::key::F25;
                break;

            case Qt::Key_division:
                mkc = morph::key::KP_DIVIDE;
                break;
            case Qt::Key_multiply:
                mkc = morph::key::KP_MULTIPLY;
                break;

            case Qt::Key_Shift:
                mkc = morph::key::LEFT_SHIFT;
                break;
            case Qt::Key_Control:
                mkc = morph::key::LEFT_CONTROL;
                break;
            case Qt::Key_Alt:
                mkc = morph::key::LEFT_ALT;
                break;
            case Qt::Key_AltGr:
                mkc = morph::key::RIGHT_ALT;
                break;
            case Qt::Key_Super_L:
                mkc = morph::key::LEFT_SUPER;
                break;
            case Qt::Key_Super_R:
                mkc = morph::key::RIGHT_SUPER;
                break;

            default:
                break;
            }
            return mkc;
        }
    }
}
