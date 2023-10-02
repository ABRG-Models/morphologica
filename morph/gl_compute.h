/*!
 * \file
 *
 * OpenGL compute shading.
 *
 * You will need to extend this class, adding your CPU land input and output attributes as well as a
 * path to your GLSL file or a const char* defining your GLSL compute shader program. See
 * examples/shadercompute.cpp
 *
 * \author Seb James
 * \date Sept 2023
 */
#pragma once

#include <morph/VisualCommon.h> // For shader processing code
#include <morph/keys.h>
#include <GLFW/glfw3.h> // GLFW is our only supported way to getting OpenGL context for morph::gl_compute
#include <GL3/gl3.h> // For GLuint and GLenum

namespace morph {

    // A default, empty compute shader.
    const char* defaultComputeShader = "#version 450\n";

    /*!
     * A gl_compute environment. I think user will extend this class to add their data structures
     * and then run with their own GLSL compute shader code.
     */
    struct gl_compute
    {
        gl_compute()
        {
            this->init_resources();
            this->init_gl();
        }
        virtual ~gl_compute()
        {
            glfwDestroyWindow (this->window);
            glfwTerminate();
        }
        void setContext() { glfwMakeContextCurrent (this->window); }
        void releaseContext() { glfwMakeContextCurrent (nullptr); }

        // You may well need to re-implement this function
        virtual void compute() {}

    private:

        void init_resources()
        {
            // Init GLFW first
            this->init_glfw();
            // Now init a window/context for compute
            this->init_window();
        }

        void init_glfw()
        {
            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }
            // Set up error callback
            glfwSetErrorCallback (morph::gl_compute::errorCallback);
            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);
            // 4.3+ required for shader compute
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
            // Tell glfw that we'd like to do anti-aliasing.
            glfwWindowHint (GLFW_SAMPLES, 4);
        }

        //! An error callback function for the GLFW windowing library
        static void errorCallback (int error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        void init_window()
        {
            this->window = glfwCreateWindow (this->win_sz[0], this->win_sz[1], this->title.c_str(), NULL, NULL);
            if (!this->window) {
                throw std::runtime_error("GLFW compute window creation failed!");
            }
            // glfwSetWindowUserPointer (this->window, this); // Necessary? Poss. not
            // Set up GLFW callbacks
            glfwSetKeyCallback (this->window, key_callback_dispatch);
            glfwSetWindowCloseCallback (this->window, window_close_callback_dispatch);
            // Lastly make the context current
            glfwMakeContextCurrent (this->window);
        }

        // Initialize OpenGL shaders, set any GL flags required
        void init_gl()
        {
            // Swap as fast as possible to compute as fast as possible
            glfwSwapInterval (0);
            this->set_shader();
            // Set GL flags
            //glEnable (GL_DEPTH_TEST); // Probably not required
            //glEnable (GL_BLEND); // spec alpha. Probably not required
            //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Probably not required
            //glDisable (GL_CULL_FACE); // Not required
            //glEnable (GL_MULTISAMPLE); // Unlikely to be required
            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

    protected:
        // Extend this with the correct glsl name (replacing Default.compute.glsl) and default GLSL
        // content (replacing morph::defaultComputeShader).
        virtual void set_shader()
        {
            // Load up the compute shader
            std::vector<morph::gl::ShaderInfo> shaders = {
                {GL_VERTEX_SHADER, "Default.compute.glsl", morph::defaultComputeShader }
            };
            this->compute_program = morph::gl::LoadShaders (shaders);
        }

    protected:
        //! The window (and OpenGL context) for this gl_compute
        GLFWwindow* window = nullptr;
        //! Window size, if needed
        morph::vec<int, 2> win_sz = { 640, 480 };
        //! The title for the object, if needed
        std::string title = "morph::gl_compute";
        //! The compute program ID.
        GLuint compute_program = 0;

    private:
        static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
        {
            gl_compute* self = static_cast<gl_compute*>(glfwGetWindowUserPointer (_window));
            if (self->key_callback (key, scancode, action, mods)) { self->compute(); }
        }
        static void window_close_callback_dispatch (GLFWwindow* _window)
        {
            gl_compute* self = static_cast<gl_compute*>(glfwGetWindowUserPointer (_window));
            self->window_close_callback();
        }

    public:
        /*
         * Generic callback handlers
         */
        using keyaction = morph::keyaction;
        using keymod = morph::keymod;
        using key = morph::key;
        // The key_callback handler uses GLFW codes, but they're in a morph header (keys.h)
        virtual bool key_callback (int _key, int scancode, int action, int mods)
        {
            bool needs_render = false;

            // Exit action
            if (_key == key::Q && (mods & keymod::CONTROL) && action == keyaction::PRESS) {
                this->signal_to_quit();
            }

            if (_key == key::H && (mods & keymod::CONTROL) && action == keyaction::PRESS) {
                // Help to stdout:
                std::cout << "Ctrl-h: Output this help to stdout\n";
            }

            this->key_callback_extra (_key, scancode, action, mods);

            return needs_render;
        }

        virtual void window_close_callback() { this->signal_to_quit(); }

        //! Extra key callback handling, making it easy for client programs to implement their own actions
        virtual void key_callback_extra (int key, int scancode, int action, int mods) {}

        //! A callback that client code can set so that it knows when user has signalled to
        //! morph::gl_compute that it's quit time.
        std::function<void()> external_quit_callback;

    protected:
        //! This internal quit function could set a 'readyToFinish' flag that your code can respond
        //! to, and calls an external callback function that you may have set up.
        void signal_to_quit()
        {
            std::cout << "User requested exit.\n";
            // 1. Set our 'readyToFinish' flag to true
            //this->readyToFinish = true;
            // 2. Call any external callback that's been set by client code
            if (this->external_quit_callback) { this->external_quit_callback(); }
        }
    };

} // namespace morph
