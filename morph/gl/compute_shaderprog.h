#pragma once
#include <type_traits>
#include <vector>
#include <string>
#include <morph/gl/util.h>
#include <morph/vec.h>
#include <morph/vvec.h>

namespace morph {
    namespace gl {

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

            // Note: I thought the setup_texture and setup_ssbo functions were only valid with a
            // single shader program. It *seems* that I can setup an SSBO with one shader program in
            // 'use' but access it in another shader program. Which would mean these functions don't
            // belong here.

            // Set up a single texture suitable for filling with values within the
            // compute shader. Note: fixed format of GL_RGBA and GL_FLOAT; could set these
            // with template params.
            void setup_texture (const GLuint image_texture_unit, unsigned int& texture_id, morph::vec<GLsizei, 2> dims)
            {
                glGenTextures (1, &texture_id); // generate one texture
                glBindTexture (GL_TEXTURE_2D, texture_id);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, dims[0], dims[1], 0, GL_RGBA, GL_FLOAT, NULL);
                glBindImageTexture (image_texture_unit, texture_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            // Set up a shader-read-only texture with the provided rgb image data
            void setup_texture (const GLuint image_texture_unit, unsigned int& texture_id,
                                morph::vec<GLsizei, 2> dims, float* rgb_data)
            {
                glGenTextures (1, &texture_id);
                glBindTexture (GL_TEXTURE_2D, texture_id);
                // Because we WRITING image data to this texture, we HAVE TO gtActiveTexture():
                glActiveTexture (GL_TEXTURE0+image_texture_unit);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                /////////////////////////////// internal format                   pixel format
                glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, dims[0], dims[1], 0, GL_RGB, GL_FLOAT, rgb_data);
                glBindImageTexture (image_texture_unit, texture_id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            // Set up a Shader Storage Buffer Object (SSBO) (with a morph::vvec)
            template<typename T>
            void setup_ssbo (const GLuint target_index, unsigned int& ssbo_id, const morph::vvec<T>& data)
            {
                glGenBuffers (1, &ssbo_id);
                glBindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
                // Mutable, re-locatable storage:
                glBufferData (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
                // Immutable storage:
                // void glBufferStorage(GLenum target​, GLsizeiptr size​, const GLvoid * data​, GLbitfield flags​);
                //glBufferStorage (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT);
                glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            // Set up a Shader Storage Buffer Object (SSBO) (morph::vec version)
            template<typename T, unsigned int N>
            void setup_ssbo (const GLuint target_index, unsigned int& ssbo_id, const morph::vec<T, N>& data)
            {
                glGenBuffers (1, &ssbo_id);
                glBindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
                glBufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), data.data(), GL_STATIC_DRAW);
                glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }
#if 0
            // Connect the program to an existing SSBO
            template<typename T>
            void connect_ssbo (const GLuint target_index, unsigned int& ssbo_id)
            {
                glGenBuffers (1, &ssbo_id);
                glBindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
                glBufferData (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
                glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }
#endif
        };

    } // namespace gl
} // namespace morph
