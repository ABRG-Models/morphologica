#pragma once

/*
 * Common code for GL functionality in morphologica programs that use multicontext GLAD headers.
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
            GLenum checkError (const char *file, int line, GladGLContext* glfn)
            {
                GLenum errorCode = 0;
#ifndef __APPLE__ // MacOS didn't like multiple calls to glGetError(); don't know why
                unsigned int ecount = 0;
                std::string error;

                while ((errorCode = glfn->GetError()) != GL_NO_ERROR) {

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
