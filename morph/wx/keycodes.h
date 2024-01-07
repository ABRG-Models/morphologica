#pragma once

#include <morph/keys.h>
#include <wx/event.h>

// WXK_ENTER WXK_APOSTROPHE and WXK_META have been omitted

namespace morph {
    namespace wx {

        // Pass in a wxWidgets key code and receive back the morph (i.e. GLFW) key code.
        constexpr int wxkey_to_morphkey(int wx_keycode)
        {
            int mkc = morph::key::unknown;
            switch (wx_keycode) {

            case WXK_SPACE:
                mkc = morph::key::space;
                break;
            case ',':
                mkc = morph::key::comma;
                break;
            case '-':
                mkc = morph::key::minus;
                break;
            case '.':
                mkc = morph::key::period;
                break;
            case '/':
                mkc = morph::key::slash;
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
                mkc = morph::key::semicolon;
                break;
            case '=':
                mkc = morph::key::equal;
                break;

            case 'a':
                mkc = morph::key::a;
                break;
            case 'B':
                mkc = morph::key::b;
                break;
            case 'C':
                mkc = morph::key::c;
                break;
            case 'D':
                mkc = morph::key::d;
                break;
            case 'E':
                mkc = morph::key::e;
                break;
            case 'F':
                mkc = morph::key::f;
                break;
            case 'G':
                mkc = morph::key::g;
                break;
            case 'H':
                mkc = morph::key::h;
                break;
            case 'I':
                mkc = morph::key::i;
                break;
            case 'J':
                mkc = morph::key::j;
                break;
            case 'K':
                mkc = morph::key::k;
                break;
            case 'L':
                mkc = morph::key::l;
                break;
            case 'M':
                mkc = morph::key::m;
                break;
            case 'N':
                mkc = morph::key::n;
                break;
            case 'O':
                mkc = morph::key::o;
                break;
            case 'P':
                mkc = morph::key::p;
                break;
            case 'Q':
                mkc = morph::key::q;
                break;
            case 'R':
                mkc = morph::key::r;
                break;
            case 'S':
                mkc = morph::key::s;
                break;
            case 'T':
                mkc = morph::key::t;
                break;
            case 'U':
                mkc = morph::key::u;
                break;
            case 'V':
                mkc = morph::key::v;
                break;
            case 'W':
                mkc = morph::key::w;
                break;
            case 'X':
                mkc = morph::key::x;
                break;
            case 'Y':
                mkc = morph::key::y;
                break;
            case 'Z':
                mkc = morph::key::z;
                break;

            case '[':
                mkc = morph::key::left_bracket;
                break;
            case '\\':
                mkc = morph::key::backslash;
                break;
            case ']':
                mkc = morph::key::right_bracket;
                break;
            case '`':
                mkc = morph::key::grave_accent;
                break;

            case WXK_ESCAPE:
                mkc = morph::key::escape;
                break;
            case WXK_TAB:
                mkc = morph::key::tab;
                break;
            case WXK_BACK:
                mkc = morph::key::backspace;
                break;
            case WXK_INSERT:
                mkc = morph::key::insert;
                break;
            case WXK_DELETE:
                mkc = morph::key::delete_key;
                break;

            case WXK_RIGHT:
                mkc = morph::key::right;
                break;
            case WXK_LEFT:
                mkc = morph::key::left;
                break;
            case WXK_DOWN:
                mkc = morph::key::down;
                break;
            case WXK_UP:
                mkc = morph::key::up;
                break;

            case WXK_PAGEUP:
                mkc = morph::key::page_up;
                break;
            case WXK_PAGEDOWN:
                mkc = morph::key::page_down;
                break;
            case WXK_HOME:
                mkc = morph::key::home;
                break;
            case WXK_END:
                mkc = morph::key::end;
                break;

            case WXK_F1:
                mkc = morph::key::f1;
                break;
            case WXK_F2:
                mkc = morph::key::f2;
                break;
            case WXK_F3:
                mkc = morph::key::f3;
                break;
            case WXK_F4:
                mkc = morph::key::f4;
                break;
            case WXK_F5:
                mkc = morph::key::f5;
                break;
            case WXK_F6:
                mkc = morph::key::f6;
                break;
            case WXK_F7:
                mkc = morph::key::f7;
                break;
            case WXK_F8:
                mkc = morph::key::f8;
                break;
            case WXK_F9:
                mkc = morph::key::f9;
                break;
            case WXK_F10:
                mkc = morph::key::f10;
                break;
            case WXK_F11:
                mkc = morph::key::f11;
                break;
            case WXK_F12:
                mkc = morph::key::f12;
                break;

            case WXK_SHIFT:
                mkc = morph::key::left_shift;
                break;
            case WXK_ALT:
                mkc = morph::key::left_alt;
                break;
            case WXK_CONTROL:
                mkc = morph::key::left_control;
                break;

            default:
                mkc = morph::key::unknown;
                break;
            }
            return mkc;
        }

    } // namespace wx
} // namespace morph
