#pragma once

/*
 * Code for shader-related GL functionality in morphologica programs.
 *
 * Note: You have to include GL3/gl3.h/GL/glext.h/GLEW3/gl31.h etc for the GL types and
 * functions BEFORE including this file.
 *
 * Author: Seb James.
 */

#include <morph/tools.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <memory>

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
            // GLenum is, in practice, a 32 bit unsigned int. The type appears not to be defined in
            // OpenGL 3.1 ES (though it does appear in 3.2 ES), so here I use unsigned int.
            unsigned int type; // rather than GLenum
            std::string filename;
            std::string compiledIn;
            GLuint shader;
        };

        // To enable debugging, set true.
        const bool debug_shaders = false;

        //! Read a shader from a file.
        std::unique_ptr<GLchar[]> ReadShader (const std::string& filename)
        {
            FILE* infile = fopen (filename.c_str(), "rb");

            if (!infile) {
                std::cerr << "Unable to open file '" << filename << "'\n";
                return NULL;
            }

            fseek (infile, 0, SEEK_END);
            int len = ftell (infile);
            fseek (infile, 0, SEEK_SET);

            std::unique_ptr<GLchar[]> source = std::make_unique<GLchar[]>(len+1);

            int itemsread = static_cast<int>(fread (source.get(), 1, len, infile));
            if (itemsread != len) { std::cerr << "Wrong number of items read!\n"; }
            fclose (infile);
            source[len] = 0;

            return source;
        }

        /*!
         * Read a default shader, stored as a const char*. ReadDefaultShader reads a
         * file: allocates some memory, copies the text into the new memory and then
         * returns a GLchar* pointer to the memory.
         */
        std::unique_ptr<GLchar[]> ReadDefaultShader (const std::string& shadercontent)
        {
            int len = shadercontent.size();
            std::unique_ptr<GLchar[]> source = std::make_unique<GLchar[]>(len+1);
            memcpy (static_cast<void*>(source.get()), static_cast<const void*>(shadercontent.c_str()), len);
            source[len] = 0;
            return source;
        }

        std::string shader_type_str (GLuint shader_type)
        {
            std::string type("unknown");
            if (shader_type == GL_VERTEX_SHADER) {
                type = "vertex";
            } else if (shader_type == GL_FRAGMENT_SHADER) {
                type = "fragment";
#ifdef GL_COMPUTE_SHADER
            } else if (shader_type == GL_COMPUTE_SHADER) {
                type = "compute";
#endif
            }
            return type;
        }

        //! Shader loading code.
        GLuint LoadShaders (const std::vector<morph::gl::ShaderInfo>& shader_info)
        {
            if (shader_info.empty()) { return 0; }

            GLuint program = glCreateProgram();

#ifdef GL_SHADER_COMPILER
            GLboolean shaderCompilerPresent = GL_FALSE;
            glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
            if (shaderCompilerPresent == GL_FALSE) {
                std::cerr << "Shader compiler NOT present!\n";
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Shader compiler present\n";
                }
            }
#endif
            for (auto entry : shader_info) {
                GLuint shader = glCreateShader (entry.type);
                entry.shader = shader;
                // Test entry.filename. If this GLSL file can be read, then do so, otherwise,
                // compile the default version specified in the ShaderInfo
                std::unique_ptr<GLchar[]> source;
                if constexpr (debug_shaders == true) {
                    std::cout << "Check file exists for " << entry.filename << std::endl;
                }
                if (morph::Tools::fileExists (entry.filename)) {
                    std::cout << "Using " << morph::gl::shader_type_str(entry.type)
                              << " shader from the file " << entry.filename << std::endl;
                    source = std::move (morph::gl::ReadShader (entry.filename));
                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Using compiled-in " << morph::gl::shader_type_str(entry.type) << " shader\n";
                    }
                    source = std::move (morph::gl::ReadDefaultShader (entry.compiledIn));
                }
                if (source == nullptr) {
                    for (auto entry : shader_info) {
                        glDeleteShader (entry.shader);
                        entry.shader = 0;
                    }
                    return 0;

                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Compiling this shader: \n" << "-----\n";
                        std::cout << source.get() << "-----\n";
                    }
                }
                GLint slen = (GLint)strlen (source.get());
                const GLchar* sptr = source.get();
                glShaderSource (shader, 1, &sptr, &slen);

                glCompileShader (shader);

                GLint shaderCompileSuccess = GL_FALSE;
                char infoLog[512];
                glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
                if (!shaderCompileSuccess) {
                    glGetShaderInfoLog(shader, 512, NULL, infoLog);
                    std::cerr << "\nShader compilation failed!";
                    std::cerr << "\n--------------------------\n\n";
                    std::cerr << source.get();
                    std::cerr << "\n\n--------------------------\n";
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
                    std::cout << "Successfully compiled a " << morph::gl::shader_type_str(entry.type) << " shader!\n";
                }
                glAttachShader (program, shader);
                glDeleteShader (shader); // Note it's correct to glDeleteShader after attaching it to program
            }

            glLinkProgram (program);

            GLint linked;
            glGetProgramiv (program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLsizei len;
                glGetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
                {
                    std::unique_ptr<GLchar[]> log = std::make_unique<GLchar[]>(len+1);
                    glGetProgramInfoLog (program, len, &len, log.get());
                    std::cerr << "Shader linking failed: " << log.get() << std::endl << "Exiting.\n";
                }
                glDeleteProgram (program);
                exit (5);
            } // else successfully linked

            return program;
        }
    } // namespace gl
} // namespace
