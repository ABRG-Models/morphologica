#pragma once

/*
 * Common code for SSBO interactions in morph programs
 *
 * Note: You have to include GL3/gl3.h/GL/glext.h/GLEW3/gl31.h etc for the GL types and
 * functions BEFORE including this file.
 *
 * Author: Seb James.
 */

#include <morph/vvec.h>
#include <morph/range.h>
#include <morph/gl/util.h>

namespace morph {
    namespace gl {
        // Map the SSBO to cpu space, then make a copy of the data into a passed-in vvec.
        //
        // ssbo_idx: The Index of the Shader Storage Buffer Object that we're reading from
        // ssbo_name: The name (really a number) of the Shader Storage Buffer Object that we're reading from
        // cpu_data: A vvec of the right size to receive the data in the SSBO into 'CPU accessible memory'
        template <typename T>
        void ssbo_copy_to_vvec (const unsigned int ssbo_idx, const unsigned int ssbo_name, morph::vvec<T>& cpu_side)
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
            // Really, it's crazy to *copy* because the data is *already in CPU
            // accessible memory* after glMapBufferRange. BUT here's the copy:
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, cpu_side.size()*sizeof(T), GL_MAP_READ_BIT));
            for (unsigned int i = 0; i < cpu_side.size(); ++i) { cpu_side[i] = cpuptr[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        template <typename T, unsigned int N>
        void ssbo_copy_to_vec (const unsigned int ssbo_idx, const unsigned int ssbo_name, morph::vec<T, N>& cpu_side)
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
            // Really, it's crazy to *copy* because the data is *already in CPU
            // accessible memory* after glMapBufferRange. BUT here's the copy:
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
            for (unsigned int i = 0; i < N; ++i) { cpu_side[i] = cpuptr[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Copy, without creating a new SSBO
        template<typename T>
        void copy_vvec_to_ssbo (const GLuint target_index, const unsigned int ssbo_id, const morph::vvec<T>& data)
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
            // Mutable, re-locatable storage:
            glBufferData (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
            // Immutable storage:
            // void glBufferStorage(GLenum target​, GLsizeiptr size​, const GLvoid * data​, GLbitfield flags​);
            //glBufferStorage (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Find the range of the data in the given Shader Storage Buffer Object
        //
        // ssbo_idx: The Index of the Shader Storage Buffer Object that we're reading from
        // ssbo_name: The name (really a number) of the Shader Storage Buffer Object that we're reading from
        // ssbo_num_elements: The number of elements of type T in the SSBO.
        template <typename T>
        morph::range<T> ssbo_get_range (const unsigned int ssbo_idx, const unsigned int ssbo_name, const unsigned int ssbo_num_elements)
        {
            morph::range<T> r;
            r.search_init();
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, ssbo_num_elements*sizeof(T), GL_MAP_READ_BIT));
            for (unsigned int i = 0; i < ssbo_num_elements; ++i) { r.update (cpuptr[i]); }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            morph::gl::Util::checkError (__FILE__, __LINE__);
            return r;
        }

    } // gl
} // morph
