#pragma once

#include <morph/Vector.h>

// For GLuint and GLenum
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include "GL3/gl3.h"
#endif

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

namespace morph {
    namespace gl {

        //! The locations for the position, normal and colour vertex attributes in the
        //! morph::Visual GLSL programs
        enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2, textureLoc = 3 };

        //! A struct to hold information about font glyph properties
        struct CharInfo
        {
            //! ID handle of the glyph texture
            unsigned int textureID;
            //! Size of glyph
            morph::Vector<int,2>  size;
            //! Offset from baseline to left/top of glyph
            morph::Vector<int,2>  bearing;
            //! Offset to advance to next glyph
            unsigned int advance;
        };

        //! A class containing a static function to check the GL errors.
        struct Util
        {
            static GLenum checkError (const char *file, int line)
            {
                GLenum errorCode = 0;
#ifndef __OSX__ // MacOS didn't like multiple calls to glGetError(); don't know why
                while ((errorCode = glGetError()) != GL_NO_ERROR) {
                    std::string error;
                    switch (errorCode) {
                    case GL_INVALID_ENUM:
                    {
                        error = "GL error: GL_INVALID_ENUM";
                        break;
                    }
                    case GL_INVALID_VALUE:
                    {
                        error = "GL error: GL_INVALID_VALUE";
                        break;
                    }
                    case GL_INVALID_OPERATION:
                    {
                        error = "GL error: GL_INVALID_OPERATION";
                        break;
                    }
                    case 1283: // Not part of GL3?
                    {
                        error = "GL error: GL_STACK_OVERFLOW";
                        break;
                    }
                    case 1284: // Not part of GL3?
                    {
                        error = "GL error: GL_STACK_UNDERFLOW";
                        break;
                    }
                    case GL_OUT_OF_MEMORY:
                    {
                        error = "GL error: GL_OUT_OF_MEMORY";
                        break;
                    }
                    case GL_INVALID_FRAMEBUFFER_OPERATION:
                    {
                        error = "GL error: GL_INVALID_FRAMEBUFFER_OPERATION";
                        break;
                    }
                    default:
                    {
                        error = "GL checkError: Unknown GL error code";
                        break;
                    }
                    }
                    std::cout << error << " | " << file << ":" << line << std::endl;
                }
#endif
                return errorCode;
            }
        };
    } // namespace gl
} // namespace
