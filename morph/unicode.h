/*!
 * \file
 *
 * Unicode text manipulation utility code
 *
 * \author Seb James
 * \date January 2022
 */

#pragma once

#include <string>

namespace morph {

    struct unicode
    {
        static std::basic_string<char32_t> fromUtf8 (const std::string& input)
        {
            std::basic_string<char32_t> utxt;

            // Fix this so that it *interprets* _txt as unicode and builds up utxt
            // accordingly. Then I only have to have char32_t here.
            char32_t uc = 0;
            int pos = 0; // byte position in uc
            for (size_t i = 0; i < input.size(); ++i) {
                // Decode UTF-8
                char c = input[i];
                if (c & 0x80) {
                    //std::cout << "UTF-8 code. char c is " << static_cast<unsigned long int>(c) << "\n";
                    // Top bit is 1, so interpret and add to c.
                    if ((c & 0xc0) == 0xc0) {
                        // Two top bits are 1 so that means start a new unicode character
                        uc = 0;
                        if ((c & 0xf0) == 0xf0) {
                            // 21 bits of data
                            // Place 3 lowest bits of c into uc in the right place
                            uc |= ((c & 0x7) << 18);
                            pos = 2;

                        } else if ((c & 0xe0) == 0xe0) {
                            // 16 bits of data
                            uc = 0;
                            // Place 4 lowest bits of c into uc in the right place
                            uc |= ((c & 0xf) << 12);
                            pos = 1;

                        } else {
                            // 11 bits of data
                            uc = 0;
                            // Place 5 lowest bits of c into uc in the right place
                            uc |= ((c & 0x1f) << 6);
                            pos = 0;
                        }
                    } else {
                        // leading code is 10b. Place 6 lowest bits of c into uc in the right place
                        if (pos >= 0) {
                            char32_t code = ( (c & 0x3f) << (pos * 6) );
                            uc |= code;
                            if (pos == 0) {
                                // uc is finished!
                                utxt.push_back (uc);
                            }
                            pos -= 1;
                        } // else error in UTF-8
                    }
                } else {
                    //std::cout << "1 ASCII char\n";
                    pos = 0;
                    utxt.push_back (static_cast<char32_t>(input[i]));
                }
            }

            return utxt;
        }

        // Convert the Unicode char32_t c into a std::string containing the
        // corrresponding UTF-8 character code sequence.
        static std::string toUtf8 (const char32_t c)
        {
            std::string rtn("");
            if (c < 0x80) {
                rtn.resize (1);
                rtn[0] = (c>>0 & 0x7f)  | 0x00;
            } else if (c < 0x800) {
                rtn.resize (2);
                rtn[0] = (c>>6 & 0x1f)  | 0xc0;
                rtn[1] = (c>>0 & 0x3f)  | 0x80;
            } else if (c < 0x10000) {
                rtn.resize (3);
                rtn[0] = (c>>12 & 0x0f) | 0xe0;
                rtn[1] = (c>>6  & 0x3f) | 0x80;
                rtn[2] = (c>>0  & 0x3f) | 0x80;
            } else if (c < 0x110000) {
                rtn.resize (4);
                rtn[0] = (c>>18 & 0x07) | 0xf0;
                rtn[1] = (c>>12 & 0x3f) | 0x80;
                rtn[2] = (c>>6  & 0x3f) | 0x80;
                rtn[3] = (c>>0  & 0x3f) | 0x80;
            }
            return rtn;
        }

        // Append a Unicode character to the end of string s as a UTF-8 code sequence
        static void append (std::string& s, const char32_t c)
        {
            std::string s1 = morph::unicode::toUtf8(c);
            s += s1;
        }
    };

} // namespace morph
