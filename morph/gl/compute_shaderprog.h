#pragma once
#include <type_traits>
#include <vector>
#include <string>
#include <sstream>
#include <cstddef>
#include <morph/gl/version.h>
#include <morph/gl/util.h>
#include <morph/vec.h>
#include <morph/vvec.h>

namespace morph {
    namespace gl {

        template<int glver>
        struct compute_shaderprog
        {
            GLuint prog_id = 0;

            // Default constructor
            compute_shaderprog() {}

            /*
             * Construct with a structure of shader files looking like this:
             *
             * std::vector<morph::gl::ShaderInfo> shaders = {
             *   {GL_COMPUTE_SHADER, "my_compute_shader.glsl", morph::defaultComputeShader }
             * };
             */
            compute_shaderprog (const std::vector<morph::gl::ShaderInfo>& shaders)
            {
                this->load_shaders (shaders);
            }

            ~compute_shaderprog()
            {
                if (this->prog_id) {
                    glDeleteProgram (this->prog_id);
                    this->prog_id = 0;
                }
            }

            void load_shaders (const std::vector<morph::gl::ShaderInfo>& shaders)
            {
                this->prog_id = morph::gl::LoadShaders (shaders);
            }

            void use() const { glUseProgram (this->prog_id); }

            // Convenience wrapper for dispatch
            void dispatch (GLuint ngrps_x, GLuint ngrps_y, GLuint ngrps_z) const
            {
                glDispatchCompute (ngrps_x, ngrps_y, ngrps_z);
                // Choices of GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, GL_SHADER_STORAGE_BARRIER_BIT or GL_ALL_BARRIER_BITS (or others).
                glMemoryBarrier (GL_ALL_BARRIER_BITS);
            }

            // Set a uniform variable into the OpenGL context associated with this shader program
            template <typename T>
            void set_uniform (const std::string& glsl_varname, const T& value)
            {
                GLint uloc = glGetUniformLocation (this->prog_id, static_cast<const GLchar*>(glsl_varname.c_str()));
                this->check_uniform_location (glsl_varname, uloc);
                if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                    if (uloc != -1) { glUniform1f (uloc, static_cast<GLfloat>(value)); }
                } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                    if (uloc != -1) { glUniform1i (uloc, static_cast<GLint>(value)); }
                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                    if (uloc != -1) { glUniform1ui (uloc, static_cast<GLuint>(value)); }
                } else {
                    []<bool flag = false>() { static_assert(flag, "Can't set that type as a uniform in an OpenGL context"); }();
                }
            }

            // Set an array into the OpenGL context
            template <typename T, std::size_t N>
            void set_uniform (const std::string& glsl_varname, const morph::vec<T, N>& value)
            {
                GLint uloc = glGetUniformLocation (this->prog_id, static_cast<const GLchar*>(glsl_varname.c_str()));
                this->check_uniform_location (glsl_varname, uloc);
                if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                    if (uloc != -1) { glUniform1fv (uloc, N, value.data()); }
                } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                    if (uloc != -1) { glUniform1iv (uloc, N, value.data()); }
                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                    if (uloc != -1) { glUniform1uiv (uloc, N, value.data()); }
                } else {
                    []<bool flag = false>() { static_assert(flag, "Can't set that type as a uniform array in an OpenGL context"); }();
                }
            }

        private:
            // Runtime check on a uniform location. If -1 throw exception. This is useful because
            // any uniform variable in a GLSL program which is not used may be compiled out and thus
            // be not 'active'. In this case, glGetUniformLocation will return -1. Our programmer
            // should be told in an exception, and either not try to set_uniform on an inactive
            // variable, or they should ensure the uniform IS actually used in their program.
            void check_uniform_location (const std::string& glsl_varname, const GLint uloc) const
            {
                if (uloc == -1) {
                    std::stringstream ee;
                    ee << "Error: glGetUniformLocation returned -1\n"
                       << "Failed to get uniform location for the ACTIVE uniform " << glsl_varname.c_str()
                       << "\n(Hint: Make sure you USE your uniform in your GLSL code)" << std::endl;
                    throw std::runtime_error (ee.str());
                }
            }
        };

    } // namespace gl
} // namespace morph
