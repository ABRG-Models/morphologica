/*!
 * \file
 *
 * Singleton to manage init/deinit of GLFW3
 *
 * \author Seb James
 * \date March 2025
 */

#pragma once

#ifndef _glfw3_h_
# define GLFW_INCLUDE_NONE
# include <GLFW/glfw3.h>
#endif

#include <iostream>
#include <stdexcept>

namespace morph {

    //! Singleton resource class for morph::Visual scenes.
    template<int glver>
    class VisualGlfw
    {
    private:
        VisualGlfw() { }
        ~VisualGlfw() { glfwTerminate(); }

        bool initialized = false;

    public:
        void init()
        {
            if (this->initialized) { return; } // as already initialized

            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }

            // Set up error callback
            glfwSetErrorCallback (morph::VisualGlfw<glver>::errorCallback);

            // The rest of this function may be right to call with each window?
            if constexpr (morph::gl::version::gles (glver) == true) {
                glfwWindowHint (GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint (GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
            }
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, morph::gl::version::major (glver));
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, morph::gl::version::minor (glver));
#ifdef __APPLE__
            glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
            // Tell glfw that we'd like to do anti-aliasing.
            glfwWindowHint (GLFW_SAMPLES, 4);

            this->initialized = true;
        }

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        VisualGlfw(const VisualGlfw<glver>&) = delete;
        VisualGlfw& operator=(const VisualGlfw<glver> &) = delete;
        VisualGlfw(VisualGlfw<glver> &&) = delete;
        VisualGlfw & operator=(VisualGlfw<glver> &&) = delete;

        //! C++11 magic statics (N2660) instance public function.
        static auto& i()
        {
            static VisualGlfw instance;
            return instance;
        }
    };

} // namespace morph
