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
#include <GL3/gl3.h>    // For GLuint and GLenum
#include <GL/glext.h>   // For GL_COMPUTE_SHADER
#include <chrono>
#include <morph/VisualDefaultShaders.h>

namespace morph {

    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    // A default, empty compute shader with a minimal layout to allow it to compile
    const char* defaultComputeShader = "#version 450 core\nlayout (local_size_x = 1) in;\n";

    /*!
     * A gl_compute environment. I think user will extend this class to add their data structures
     * and then run with their own GLSL compute shader code.
     */
    template <int gl_version_major = 4, int gl_version_minor = 5>
    struct gl_compute
    {
        gl_compute() { this->t0 = sc::now(); }
        ~gl_compute()
        {
            glfwDestroyWindow (this->window);
            if (this->compute_program) {
                // hmm - there's also glDeleteProgram. you can actually glDeleteShader
                // after linking the shader to the program. FIXME.
                glDeleteShader (this->compute_program);
                this->compute_program = 0;
            }
            glfwTerminate();
        }

        void init()
        {
            this->init_resources();
            this->init_gl();
        }

        void setContext()
        {
            glfwMakeContextCurrent (this->window);
            glfwSwapInterval (0);
        }
        void releaseContext() { glfwMakeContextCurrent (nullptr); }

        // You may well need to re-implement this function
        virtual void compute() {}

        void keepOpen()
        {
            while (this->readyToFinish == false) {
                glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
                this->render();
            }
        }

        //! You may wish to render the result of your compute. This method can be overridden.
        virtual void render()
        {
            glfwSwapBuffers (this->window);
        }

        //! Set to true when the program should end
        bool readyToFinish = false;

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
            glfwSetErrorCallback (morph::gl_compute<>::errorCallback);
            // See https://www.glfw.org/docs/latest/monitor_guide.html
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(primary, &xscale, &yscale);
            // 4.3+ required for shader compute
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, gl_version_major);
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, gl_version_minor);
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
            // Set up GLFW callbacks
            glfwSetWindowUserPointer (this->window, this); // Bind window to this object. Required for callbacks
            glfwSetKeyCallback (this->window, key_callback_dispatch);
            glfwSetWindowCloseCallback (this->window, window_close_callback_dispatch);

            // Lastly make the context current
            glfwMakeContextCurrent (this->window);
            glfwSwapInterval (0);
        }

        // Initialize OpenGL shaders, set any GL flags required
        void init_gl()
        {
            // Swap as fast as possible to compute as fast as possible
            glfwSwapInterval (0);

            // Check GL_MAX_COMPUTE_WORK_GROUP_COUNT and send to std::out
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &this->max_compute_work_group_count[0]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &this->max_compute_work_group_count[1]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &this->max_compute_work_group_count[2]);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNTS (x, y, z): " << this->max_compute_work_group_count << std::endl;

            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &this->max_compute_work_group_size[0]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &this->max_compute_work_group_size[1]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &this->max_compute_work_group_size[2]);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE (x, y, z): " << this->max_compute_work_group_size << std::endl;

            glGetInteger64v (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &this->max_compute_work_group_invocations);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << this->max_compute_work_group_invocations << std::endl;

            load_shaders();

            // No need to set any GL flags (though a derived class may need to if it is to render any graphics)

            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

    public:
        // Add an implementation of load_shaders looking something like:
        //
        // void load_shaders() finals
        // {
        //   {
        //     std::vector<morph::gl::ShaderInfo> shaders = {
        //     {GL_COMPUTE_SHADER, "Default.compute.glsl", morph::defaultComputeShader }
        //   };
        //   this->compute_program = morph::gl::LoadShaders (shaders);
        // }
        //
        // Here "Default.compute.glsl" is the path to a file containing the GLSL
        // code. morph::defaultComputeShader is a const char* of some default GLSL code
        // text that will be used if the file cannot be accessed.
        //
        virtual void load_shaders() = 0;

    protected:
        //! The window (and OpenGL context) for this gl_compute
        GLFWwindow* window = nullptr;
        //! Window size, if needed
        morph::vec<int, 2> win_sz = { 640, 480 };
        //! The title for the object, if needed
        std::string title = "morph::gl_compute";
        //! The compute program ID.
        GLuint compute_program = 0;

        // GL_MAX_COMPUTE_WORK_GROUP_COUNT, _GROUP_SIZE and _INVOCATIONS as queried from OpenGL
        morph::vec<GLint64, 3> max_compute_work_group_count = {-1,-1,-1};
        morph::vec<GLint64, 3> max_compute_work_group_size = {-1,-1,-1};
        GLint64 max_compute_work_group_invocations = -1;

        // For frame count timing
        static constexpr unsigned int nframes = 1000;
        static constexpr double nframes_d = nframes;
        static constexpr double nframes_d_us = nframes_d * 1000000;
        unsigned int frame_count = 0;
        sc::time_point t0, t1;

        // Measure the time to execute nframes frames and output an FPS message. Client
        // code has to call this with every call to compute() to get the measurement
        // (though its use is entirely optional).
        void measure_compute()
        {
            if ((frame_count++ % nframes) == 0) {
                this->t1 = sc::now();
                sc::duration t_d = t1 - t0;
                double s_per_frame = duration_cast<microseconds>(t_d).count() / nframes_d_us;
                std::cout << "FPS: " << 1.0/s_per_frame << std::endl;
                this->t0 = this->t1;
            }
        }

    private:
        static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
        {
            gl_compute* self = static_cast<gl_compute*>(glfwGetWindowUserPointer (_window));
            if (self->key_callback (key, scancode, action, mods)) {
                std::cout << "key_callback returned\n";
                self->compute();
            }
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
                std::cout << "Ctrl-h: Output this help to stdout\n";
                std::cout << "Ctrl-q: Quit\n";
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
            this->readyToFinish = true;
            // 2. Call any external callback that's been set by client code
            if (this->external_quit_callback) { this->external_quit_callback(); }
        }
    };

} // namespace morph
