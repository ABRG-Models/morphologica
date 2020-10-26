#pragma once

#include <morph/Vector.h>

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

namespace morph {
    //! The locations for the position, normal and colour vertex attributes in the
    //! morph::Visual GLSL programs
    enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2, textureLoc = 3 };

    //! A struct to hold information about font glyph properties
    struct Character
    {
        //! ID handle of the glyph texture
        unsigned int TextureID;
        //! Size of glyph
        morph::Vector<int,2>  Size;
        //! Offset from baseline to left/top of glyph
        morph::Vector<int,2>  Bearing;
        //! Offset to advance to next glyph
        unsigned int Advance;
    };
} // namespace
