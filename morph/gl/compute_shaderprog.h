#pragma once
#include <type_traits>
#include <vector>
#include <string>
#include <morph/gl/util.h>
#include <morph/vec.h>
#include <morph/vvec.h>

namespace morph {
    namespace gl {

        template<int gl_version_major = 4, int gl_version_minor = 5, bool gles = false>
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
        };

    } // namespace gl
} // namespace morph
