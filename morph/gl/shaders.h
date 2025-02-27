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
#include <filesystem>

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
            if (!std::filesystem::is_regular_file (filename)) { // restrict to regular files
                std::cerr << "'" << filename << "' is not a regular file\n";
                return nullptr;
            }
            size_t len = std::filesystem::file_size (filename);
            std::unique_ptr<GLchar[]> source = std::make_unique<GLchar[]>(len + 1);
            std::ifstream fin (filename.c_str(), std::ios::in);
            if (!fin.is_open()) {
                std::cerr << "Unable to open file '" << filename << "'\n";
                return nullptr;
            }
            if (!fin.eof()) { fin.read (source.get(), len); }
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
            std::size_t len = shadercontent.size();
            std::unique_ptr<GLchar[]> source = std::make_unique<GLchar[]>(len + 1);
            std::memcpy (static_cast<void*>(source.get()), static_cast<const void*>(shadercontent.c_str()), len);
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
        GLuint LoadShaders (const std::vector<morph::gl::ShaderInfo>& shader_info
#ifdef GLAD_OPTION_GL_MX
                            , GladGLContext* glfn
#endif
            )
        {
            if (shader_info.empty()) { return 0; }

#ifdef GLAD_OPTION_GL_MX
            GLuint program = glfn->CreateProgram();
#else
            GLuint program = glCreateProgram();
#endif

#ifdef GL_SHADER_COMPILER
            GLboolean shaderCompilerPresent = GL_FALSE;
# ifdef GLAD_OPTION_GL_MX
            glfn->GetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
# else
            glGetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
# endif
            if (shaderCompilerPresent == GL_FALSE) {
                std::cerr << "Shader compiler NOT present!\n";
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Shader compiler present\n";
                }
            }
#endif
            for (auto entry : shader_info) {
#ifdef GLAD_OPTION_GL_MX
                GLuint shader = glfn->CreateShader (entry.type);
#else
                GLuint shader = glCreateShader (entry.type);
#endif
                entry.shader = shader;
                // Test entry.filename. If this GLSL file can be read, then do so, otherwise,
                // compile the default version specified in the ShaderInfo
                std::unique_ptr<GLchar[]> source;
                if constexpr (debug_shaders == true) {
                    std::cout << "Check file exists for " << entry.filename << std::endl;
                }
                if (morph::tools::fileExists (entry.filename)) {
                    std::cout << "Using " << morph::gl::shader_type_str(entry.type)
                              << " shader from the file " << entry.filename << std::endl;
                    source = morph::gl::ReadShader (entry.filename);
                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Using compiled-in " << morph::gl::shader_type_str(entry.type) << " shader\n";
                    }
                    source = morph::gl::ReadDefaultShader (entry.compiledIn);
                }
                if (source == nullptr) {
                    for (auto entry : shader_info) {
#ifdef GLAD_OPTION_GL_MX
                        glfn->DeleteShader (entry.shader);
#else
                        glDeleteShader (entry.shader);
#endif
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
#ifdef GLAD_OPTION_GL_MX
                glfn->ShaderSource (shader, 1, &sptr, &slen);
                glfn->CompileShader (shader);
#else
                glShaderSource (shader, 1, &sptr, &slen);
                glCompileShader (shader);
#endif
                GLint shaderCompileSuccess = GL_FALSE;
                char infoLog[512];
#ifdef GLAD_OPTION_GL_MX
                glfn->GetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
#else
                glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
#endif
                if (!shaderCompileSuccess) {
#ifdef GLAD_OPTION_GL_MX
                    glfn->GetShaderInfoLog(shader, 512, NULL, infoLog);
#else
                    glGetShaderInfoLog(shader, 512, NULL, infoLog);
#endif
                    std::cerr << "\nShader compilation failed!";
                    std::cerr << "\n--------------------------\n\n";
                    std::cerr << source.get();
                    std::cerr << "\n\n--------------------------\n";
                    std::cerr << infoLog << std::endl;
                    std::cerr << "Exiting.\n";
                    exit (2);
                }

                // Test glGetError:
#ifdef GLAD_OPTION_GL_MX
                GLenum shaderError = glfn->GetError();
#else
                GLenum shaderError = glGetError();
#endif
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
#ifdef GLAD_OPTION_GL_MX
                glfn->AttachShader (program, shader);
                glfn->DeleteShader (shader); // Note it's correct to glDeleteShader after attaching it to program
#else
                glAttachShader (program, shader);
                glDeleteShader (shader); // Note it's correct to glDeleteShader after attaching it to program
#endif
            }

            GLint linked = 0;
#ifdef GLAD_OPTION_GL_MX
            glfn->LinkProgram (program);
            glfn->GetProgramiv (program, GL_LINK_STATUS, &linked);
#else
            glLinkProgram (program);
            glGetProgramiv (program, GL_LINK_STATUS, &linked);
#endif
            if (!linked) {
                GLsizei len = 0;
#ifdef GLAD_OPTION_GL_MX
                glfn->GetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
                {
                    std::unique_ptr<GLchar[]> log = std::make_unique<GLchar[]>(len+1);
                    glfn->GetProgramInfoLog (program, len, &len, log.get());
                    std::cerr << "Shader linking failed: " << log.get() << std::endl << "Exiting.\n";
                }
                glfn->DeleteProgram (program);
#else
                glGetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
                {
                    std::unique_ptr<GLchar[]> log = std::make_unique<GLchar[]>(len+1);
                    glGetProgramInfoLog (program, len, &len, log.get());
                    std::cerr << "Shader linking failed: " << log.get() << std::endl << "Exiting.\n";
                }
                glDeleteProgram (program);
#endif
                exit (5);
            } // else successfully linked

            return program;
        }
    } // namespace gl
} // namespace
