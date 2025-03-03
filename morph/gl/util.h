#pragma once

/*
 * Common code for GL functionality in morphologica programs.
 *
 * Note: You have to include a header like gl3.h or glext.h etc for the GL types and
 * functions BEFORE including this file.
 *
 * Author: Seb James.
 */

#include <stdexcept>
#include <string>
#include <iostream>

namespace morph {
    namespace gl {
        //! A GL error checking function. The additional namespace was a class, but didn't need to be.
        namespace Util {
            GLenum checkError (const char *file, int line)
            {
                GLenum errorCode = 0;
#ifndef __OSX__ // MacOS didn't like multiple calls to glGetError(); don't know why
                unsigned int ecount = 0;
                std::string error;

                while ((errorCode = glGetError()) != GL_NO_ERROR) {

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
                    ++ecount;
                }
                if (ecount) { throw std::runtime_error (error); }
#endif
                return errorCode;
            }
        } // namespace Util
    } // namespace gl
} // namespace
