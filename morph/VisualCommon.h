#pragma once

/*
 * Common code for GL functionality in morphologica programs.
 *
 * Author: Seb James.
 */

#include <morph/vec.h>
#include <morph/tools.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

// For GLuint and GLenum
#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif

namespace morph {

    namespace gl {

        /*!
         * Data structure for shader info.
         *
         * LoadShaders() takes an array of ShaderFile structures, each of which contains the
         * type of the shader, and a pointer a C-style character string (i.e., a
         * NULL-terminated array of characters) containing the filename of a GLSL file to
         * use, and another pointer to the compiled-in version of the shader.
         *
         * The array of structures is terminated by a final Shader with
         * the "type" field set to GL_NONE.
         *
         * LoadShaders() returns the shader program value (as returned by
         * glCreateProgram()) on success, or zero on failure.
         */
        struct ShaderInfo
        {
            GLenum type;
            const char* filename;
            const char* compiledIn;
            GLuint shader;
        };

        // To enable debugging, set true.
        const bool debug_shaders = false;

        //! Read a shader from a file.
        const GLchar* ReadShader (const char* filename)
        {
            FILE* infile = fopen (filename, "rb");

            if (!infile) {
                std::cerr << "Unable to open file '" << filename << "'\n";
                return NULL;
            }

            fseek (infile, 0, SEEK_END);
            int len = ftell (infile);
            fseek (infile, 0, SEEK_SET);

            GLchar* source = new GLchar[len+1];

            int itemsread = static_cast<int>(fread (source, 1, len, infile));
            if (itemsread != len) { std::cerr << "Wrong number of items read!\n"; }
            fclose (infile);

            source[len] = 0;

            return const_cast<const GLchar*>(source);
        }

        /*!
         * Read a default shader, stored as a const char*. ReadDefaultShader reads a
         * file: allocates some memory, copies the text into the new memory and then
         * returns a GLchar* pointer to the memory.
         */
        const GLchar* ReadDefaultShader (const char* shadercontent)
        {
            int len = strlen (shadercontent);
            GLchar* source = new GLchar[len+1];
            memcpy (static_cast<void*>(source), static_cast<const void*>(shadercontent), len);
            source[len] = 0;
            return const_cast<const GLchar*>(source);
        }

        //! Shader loading code.
        GLuint LoadShaders (std::vector<morph::gl::ShaderInfo>& shader_info)
        {
            if (shader_info.empty()) { return 0; }

            GLuint program = glCreateProgram();

            GLboolean shaderCompilerPresent = GL_FALSE;
            glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
            if (shaderCompilerPresent == GL_FALSE) {
                std::cerr << "Shader compiler NOT present!\n";
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Shader compiler present\n";
                }
            }

            for (auto& entry : shader_info) {
                GLuint shader = glCreateShader (entry.type);
                entry.shader = shader;
                // Test entry.filename. If this GLSL file can be read, then do so, otherwise,
                // compile the default version from VisualDefaultShaders.h
                const GLchar* source;
                if (morph::Tools::fileExists (std::string(entry.filename))) {
                    std::cout << "Using shader from the file " << entry.filename << std::endl;
                    source = morph::gl::ReadShader (entry.filename);
                } else {
                    if (entry.type == GL_VERTEX_SHADER) {
                        if constexpr (debug_shaders == true) {
                            std::cout << "Using compiled-in vertex shader\n";
                        }
                        source = morph::gl::ReadDefaultShader (entry.compiledIn);
                    } else if (entry.type == GL_FRAGMENT_SHADER) {
                        if constexpr (debug_shaders == true) {
                            std::cout << "Using compiled-in fragment shader\n";
                        }
                        source = morph::gl::ReadDefaultShader (entry.compiledIn);
                    } else {
                        std::cerr << "morph::gl::LoadShaders: Unknown shader entry->type...\n";
                        source = NULL;
                    }
                }
                if (source == NULL) {
                    for (auto& entry : shader_info) {
                        glDeleteShader (entry.shader);
                        entry.shader = 0;
                    }
                    return 0;

                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Compiling this shader: \n" << "-----\n";
                        std::cout << source << "-----\n";
                    }
                }
                GLint slen = (GLint)strlen (source);
                glShaderSource (shader, 1, &source, &slen);
                delete [] source;

                glCompileShader (shader);

                GLint shaderCompileSuccess = GL_FALSE;
                char infoLog[512];
                glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
                if (!shaderCompileSuccess) {
                    glGetShaderInfoLog(shader, 512, NULL, infoLog);
                    std::cerr << "\nShader compilation failed!";
                    std::cerr << "\n--------------------------\n";
                    std::cerr << infoLog << std::endl;
                    std::cerr << "Exiting.\n";
                    exit (2);
                }

                // Test glGetError:
                GLenum shaderError = glGetError();
                if (shaderError == GL_INVALID_VALUE) {
                    std::cerr << "Shader compilation resulted in GL_INVALID_VALUE\n";
                    exit (3);
                } else if (shaderError == GL_INVALID_OPERATION) {
                    std::cerr << "Shader compilation resulted in GL_INVALID_OPERATION\n";
                    exit (4);
                } // shaderError is 0

                if constexpr (debug_shaders == true) {
                    if (entry.type == GL_VERTEX_SHADER) {
                        std::cout << "Successfully compiled vertex shader!\n";
                    } else if (entry.type == GL_FRAGMENT_SHADER) {
                        std::cout << "Successfully compiled fragment shader!\n";
                    } else {
                        std::cout << "Successfully compiled shader!\n";
                    }
                }
                glAttachShader (program, shader);
            }

            glLinkProgram (program);

            GLint linked;
            glGetProgramiv (program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLsizei len;
                glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );
                GLchar* log = new GLchar[len+1];
                glGetProgramInfoLog( program, len, &len, log );
                std::cerr << "Shader linking failed: " << log << std::endl << "Exiting.\n";
                delete [] log;
                for (auto& entry : shader_info) {
                    glDeleteShader (entry.shader);
                    entry.shader = 0;
                }
                exit (5);
            } // else successfully linked

            return program;
        }

        // A container struct for the shader program identifiers used in a morph::Visual. Separate
        // from morph::Visual so that it can be used in morph::VisualModel as well, which does not
        // #include morph/Visual.h.
        struct visual_shaderprogs
        {
            //! An OpenGL shader program for graphical objects
            GLuint gprog = 0;
            //! A text shader program, which uses textures to draw text on quads.
            GLuint tprog = 0;
        };

        //! The locations for the position, normal and colour vertex attributes in the
        //! morph::Visual GLSL programs
        enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2, textureLoc = 3 };

        //! A struct to hold information about font glyph properties
        struct CharInfo
        {
            //! ID handle of the glyph texture
            unsigned int textureID;
            //! Size of glyph
            morph::vec<int,2>  size;
            //! Offset from baseline to left/top of glyph
            morph::vec<int,2>  bearing;
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
        };
    } // namespace gl
} // namespace
