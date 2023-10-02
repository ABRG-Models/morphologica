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
#include <GL/glext.h> // For GL_COMPUTE_SHADER

#include <morph/VisualDefaultShaders.h>

namespace morph {

    // A default, empty compute shader with a minimal layout to allow it to compile
    const char* defaultComputeShader = "#version 450 core\nlayout (local_size_x = 1) in;\n";

    /*!
     * A gl_compute environment. I think user will extend this class to add their data structures
     * and then run with their own GLSL compute shader code.
     */
    struct gl_compute
    {
        gl_compute(){}
        ~gl_compute()
        {
            glfwDestroyWindow (this->window);
            glfwTerminate();
        }

        void init()
        {
            this->init_resources();
            this->init_gl();
        }

        void setContext() { glfwMakeContextCurrent (this->window); }
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
            // Set up GLFW callbacks
            glfwSetWindowUserPointer (this->window, this); // Bind window to this object. Required for callbacks
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

            // Check GL_MAX_COMPUTE_WORK_GROUP_COUNT and send to std::out
            morph::vec<GLint64, 3> wgcount = { -1, -1, -1 };
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &wgcount[0]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &wgcount[1]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &wgcount[2]);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNTS (x, y, z): " << wgcount << std::endl;

            morph::vec<GLint64, 3> wgsize = { -1, -1, -1 };
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &wgsize[0]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &wgsize[1]);
            glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &wgsize[2]);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE (x, y, z): " << wgsize << std::endl;

            GLint64 wginvocations = -1;
            glGetInteger64v (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &wginvocations);
            std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << wginvocations << std::endl;

            load_shaders();

            // Set GL flags
            //glEnable (GL_DEPTH_TEST); // Probably not required
            //glEnable (GL_BLEND); // spec alpha. Probably not required
            //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Probably not required
            //glDisable (GL_CULL_FACE); // Not required
            //glEnable (GL_MULTISAMPLE); // Unlikely to be required

            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

    public:
        // Extend this with the correct glsl name (replacing Default.compute.glsl) and the default
        // GLSL:
        //
        // virtual void load_shaders()
        // {
        //   {
        //     std::vector<morph::gl::ShaderInfo> shaders = {
        //     {GL_COMPUTE_SHADER, "Default.compute.glsl", morph::defaultComputeShader }
        //   };
        //   this->compute_program = morph::gl::LoadShaders (shaders);
        // }
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
