
#pragma once

#include <morph/keys.h>
#include <wx/event.h>

namespace morph {
    namespace wx {

        // Pass in a wxWidgets key code and receive back the morph (i.e. GLFW) key code.
        constexpr int wxkey_to_morphkey(int wx_keycode)
        {
            int mkc = morph::key::UNKNOWN;
            switch (wx_keycode) {

            case WXK_SPACE:
                mkc = morph::key::SPACE;
                break;
//            case WXK_APOSTROPHE:
//                mkc = morph::key::APOSTROPHE;
                break;
            case ',':
                mkc = morph::key::COMMA;
                break;
            case '-':
                mkc = morph::key::MINUS;
                break;
            case '.':
                mkc = morph::key::PERIOD;
                break;
            case '/':
                mkc = morph::key::SLASH;
                break;

            case '0':
                mkc = morph::key::n0;
                break;
            case '1':
                mkc = morph::key::n1;
                break;
            case '2':
                mkc = morph::key::n2;
                break;
            case '3':
                mkc = morph::key::n3;
                break;
            case '4':
                mkc = morph::key::n4;
                break;
            case '5':
                mkc = morph::key::n5;
                break;
            case '6':
                mkc = morph::key::n6;
                break;
            case '7':
                mkc = morph::key::n7;
                break;
            case '8':
                mkc = morph::key::n8;
                break;
            case '9':
                mkc = morph::key::n9;
                break;

            case ';':
                mkc = morph::key::SEMICOLON;
                break;
            case '=':
                mkc = morph::key::EQUAL;
                break;

            case 'A':
                mkc = morph::key::A;
                break;
            case 'B':
                mkc = morph::key::B;
                break;
            case 'C':
                mkc = morph::key::C;
                break;
            case 'D':
                mkc = morph::key::D;
                break;
            case 'E':
                mkc = morph::key::E;
                break;
            case 'F':
                mkc = morph::key::F;
                break;
            case 'G':
                mkc = morph::key::G;
                break;
            case 'H':
                mkc = morph::key::H;
                break;
            case 'I':
                mkc = morph::key::I;
                break;
            case 'J':
                mkc = morph::key::J;
                break;
            case 'K':
                mkc = morph::key::K;
                break;
            case 'L':
                mkc = morph::key::L;
                break;
            case 'M':
                mkc = morph::key::M;
                break;
            case 'N':
                mkc = morph::key::N;
                break;
            case 'O':
                mkc = morph::key::O;
                break;
            case 'P':
                mkc = morph::key::P;
                break;
            case 'Q':
                mkc = morph::key::Q;
                break;
            case 'R':
                mkc = morph::key::R;
                break;
            case 'S':
                mkc = morph::key::S;
                break;
            case 'T':
                mkc = morph::key::T;
                break;
            case 'U':
                mkc = morph::key::U;
                break;
            case 'V':
                mkc = morph::key::V;
                break;
            case 'W':
                mkc = morph::key::W;
                break;
            case 'X':
                mkc = morph::key::X;
                break;
            case 'Y':
                mkc = morph::key::Y;
                break;
            case 'Z':
                mkc = morph::key::Z;
                break;

            case '[':
                mkc = morph::key::LEFT_BRACKET;
                break;
            case '\\':
                mkc = morph::key::BACKSLASH;
                break;
            case ']':
                mkc = morph::key::RIGHT_BRACKET;
                break;
            case '`':
                mkc = morph::key::GRAVE_ACCENT;
                break;

            case WXK_ESCAPE:
                mkc = morph::key::ESCAPE;
                break;
//            case WXK_ENTER:
//                mkc = morph::key::ENTER;
//                break;
            case WXK_TAB:
                mkc = morph::key::TAB;
                break;
            case WXK_BACK:
                mkc = morph::key::BACKSPACE;
                break;
            case WXK_INSERT:
                mkc = morph::key::INSERT;
                break;
            case WXK_DELETE:
                mkc = morph::key::DELETE;
                break;

            case WXK_RIGHT:
                mkc = morph::key::RIGHT;
                break;
            case WXK_LEFT:
                mkc = morph::key::LEFT;
                break;
            case WXK_DOWN:
                mkc = morph::key::DOWN;
                break;
            case WXK_UP:
                mkc = morph::key::UP;
                break;

            case WXK_PAGEUP:
                mkc = morph::key::PAGE_UP;
                break;
            case WXK_PAGEDOWN:
                mkc = morph::key::PAGE_DOWN;
                break;
            case WXK_HOME:
                mkc = morph::key::HOME;
                break;
            case WXK_END:
                mkc = morph::key::END;
                break;

            case WXK_F1:
                mkc = morph::key::F1;
                break;
            case WXK_F2:
                mkc = morph::key::F2;
                break;
            case WXK_F3:
                mkc = morph::key::F3;
                break;
            case WXK_F4:
                mkc = morph::key::F4;
                break;
            case WXK_F5:
                mkc = morph::key::F5;
                break;
            case WXK_F6:
                mkc = morph::key::F6;
                break;
            case WXK_F7:
                mkc = morph::key::F7;
                break;
            case WXK_F8:
                mkc = morph::key::F8;
                break;
            case WXK_F9:
                mkc = morph::key::F9;
                break;
            case WXK_F10:
                mkc = morph::key::F10;
                break;
            case WXK_F11:
                mkc = morph::key::F11;
                break;
            case WXK_F12:
                mkc = morph::key::F12;
                break;

            case WXK_SHIFT:
                mkc = morph::key::LEFT_SHIFT;
                break;
            case WXK_ALT:
                mkc = morph::key::LEFT_ALT;
                break;
            case WXK_CONTROL:
                mkc = morph::key::LEFT_CONTROL;
                break;
//            case WXK_META:
//                mkc = morph::key::LEFT_SUPER;
//                break;

            default:
                mkc = morph::key::UNKNOWN;
                break;
            }
            return mkc;
        }

    } // namespace wx
} // namespace morph
