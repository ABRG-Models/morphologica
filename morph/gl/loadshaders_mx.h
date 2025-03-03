/*
 * Multiple context-aware GL LoadShaders
 */

#pragma once

#include <morph/gl/shaders.h>

#include <morph/tools.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <memory>

namespace morph {

    namespace gl {

        //! Shader loading code.
        GLuint LoadShadersMX (const std::vector<morph::gl::ShaderInfo>& shader_info, GladGLContext* glfn)
        {
            if (shader_info.empty()) { return 0; }

            GLuint program = glfn->CreateProgram();

#ifdef GL_SHADER_COMPILER
            GLboolean shaderCompilerPresent = GL_FALSE;
            glfn->GetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
            if (shaderCompilerPresent == GL_FALSE) {
                std::cerr << "Shader compiler NOT present!\n";
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Shader compiler present\n";
                }
            }
#endif
            for (auto entry : shader_info) {
                GLuint shader = glfn->CreateShader (entry.type);
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
                        glfn->DeleteShader (entry.shader);
                        entry.shader = 0;
                    }
                    return 0;

                } else {
                    if constexpr (debug_shaders == true) {
                        std::cout << "Compiling this shader: \n" << "-----\n";
                        std::cout << source.get() << "-----\n";
                    }
                }
                GLint slen = (GLint)std::strlen (source.get());
                const GLchar* sptr = source.get();
                glfn->ShaderSource (shader, 1, &sptr, &slen);
                glfn->CompileShader (shader);
                GLint shaderCompileSuccess = GL_FALSE;
                char infoLog[512];
                glfn->GetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
                if (!shaderCompileSuccess) {
                    glfn->GetShaderInfoLog(shader, 512, NULL, infoLog);
                    std::cerr << "\nShader compilation failed!";
                    std::cerr << "\n--------------------------\n\n";
                    std::cerr << source.get();
                    std::cerr << "\n\n--------------------------\n";
                    std::cerr << infoLog << std::endl;
                    std::cerr << "Exiting.\n";
                    exit (2);
                }

                // Test glGetError:
                GLenum shaderError = glfn->GetError();
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
                glfn->AttachShader (program, shader);
                glfn->DeleteShader (shader); // Note it's correct to glDeleteShader after attaching it to program
            }

            GLint linked = 0;
            glfn->LinkProgram (program);
            glfn->GetProgramiv (program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLsizei len = 0;
                glfn->GetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
                {
                    std::unique_ptr<GLchar[]> log = std::make_unique<GLchar[]>(len+1);
                    glfn->GetProgramInfoLog (program, len, &len, log.get());
                    std::cerr << "Shader linking failed: " << log.get() << std::endl << "Exiting.\n";
                }
                glfn->DeleteProgram (program);
                exit (5);
            } // else successfully linked

            return program;
        }
    } // namespace gl
} // namespace
