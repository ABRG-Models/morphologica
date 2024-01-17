/*!
 * \file
 *
 * OpenGL compute shading. This is a 'manager class' for compute shading.
 *
 * You will need to extend this class, adding your CPU land input and output attributes as well as a
 * path to your GLSL file or a const char* defining your GLSL compute shader program. See
 * examples/shadercompute.cpp
 *
 * \author Seb James
 * \date Sept 2023
 */
#pragma once

// We *don't* want to include the GL headers here, because the correct headers may be
// GL3/gl.h and GL/glext.h for OpenGL 4.3+ OR GLES3/gl31.h and GLES3/gl3ext.h for OpenGL
// 3.1 ES. So the client code should include the correct headers before #including this
// file.

#include <morph/gl/version.h>
#include <morph/gl/util.h>
#include <morph/gl/shaders.h>
#include <morph/gl/compute_shaderprog.h> // A compute-shader class
#include <morph/keys.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // GLFW is our only supported way to getting OpenGL context for morph::gl_compute

#include <chrono>
#include <morph/VisualDefaultShaders.h>

namespace morph {
    namespace gl {

        using namespace std::chrono;
        using sc = std::chrono::steady_clock;

        // A default shader which won't compile (better than having an empty no-op default shader)
        const char* defaultComputeShader = "This is an intentionally non-compiling non-shader\n";

        // You may wish to pass a compiled-in shader that will fail, so that your system MUST find the
        // file-based shader in morph::gl::LoadShaders.
        const char* nonCompilingComputeShader = "This is an intentionally non-compiling non-shader\n";

        /*!
         * A gl compute environment. I think user will extend this class to add their data structures
         * and then run with their own GLSL compute shader code.
         */
        template <int glver = morph::gl::version_4_5>
        struct compute_manager
        {
            compute_manager() { this->t0 = sc::now(); }
            ~compute_manager()
            {
                glfwDestroyWindow (this->window);
                glfwTerminate();
            }

            //! Init GLFW and then the GLFW window. What if you want to set window width
            //! based on GLFW info such as this->workarea_width? In that case, instead
            //! of calling this do-it-all init function, just call the init functions separately:
            //! init_glfw(); do_stuff(); init_window(); init_gl();
            void init()
            {
                // Init GLFW first
                this->init_glfw();
                // Now init a window/context for compute
                this->init_window();
                // Finally init GL
                this->init_gl();
            }

            void setContext()
            {
                glfwMakeContextCurrent (this->window);
                glfwSwapInterval (0);
            }
            void releaseContext() { glfwMakeContextCurrent (nullptr); }

            // You will need to re-implement this function
            virtual void compute() = 0;

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

        protected:

            void init_glfw()
            {
                if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }
                // Set up error callback
                glfwSetErrorCallback (morph::gl::compute_manager<glver>::errorCallback);
                // See https://www.glfw.org/docs/latest/monitor_guide.html
                GLFWmonitor* primary = glfwGetPrimaryMonitor();
                glfwGetMonitorContentScale (primary, &this->monitor_xscale, &this->monitor_yscale);
                glfwGetMonitorWorkarea (primary, &this->workarea_xpos, &this->workarea_ypos, &this->workarea_width, &this->workarea_height);
                // 4.3+ or 3.1ES+ are required for shader compute
                if constexpr (morph::gl::version::gles (glver) == true) {
                    glfwWindowHint (GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                    glfwWindowHint (GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
                }
                glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, morph::gl::version::major (glver));
                glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, morph::gl::version::minor (glver));
                morph::gl::Util::checkError (__FILE__, __LINE__);
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
                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

            // Initialize OpenGL shaders, set any GL flags required
            void init_gl()
            {
                // Swap as fast as possible to compute as fast as possible
                glfwSwapInterval (0);

                unsigned char* glv = (unsigned char*)glGetString(GL_VERSION);
                std::cout << "compute_manager<" << morph::gl::version::vstring (glver)
                          << "> running on OpenGL Version " << glv << std::endl;

                // Store, and also output to stdout, some info about the GL resources available
                glGetIntegerv (GL_MAX_COMPUTE_ATOMIC_COUNTERS, &this->max_compute_atomic_counters);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_ATOMIC_COUNTERS: " << this->max_compute_atomic_counters << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, &this->max_compute_atomic_counters_buffers);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS: " << this->max_compute_atomic_counters_buffers << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &this->max_compute_shader_storage_blocks);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS: " << this->max_compute_shader_storage_blocks << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &this->max_compute_texture_image_units);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS: " << this->max_compute_texture_image_units << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_UNIFORM_BLOCKS, &this->max_compute_uniform_blocks);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_UNIFORM_BLOCKS: " << this->max_compute_uniform_blocks << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_UNIFORM_COMPONENTS, &this->max_compute_uniform_components);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_UNIFORM_COMPONENTS: " << this->max_compute_uniform_components << std::endl;

                glGetInteger64v (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &this->max_compute_work_group_invocations);
                std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << this->max_compute_work_group_invocations << std::endl;

                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &this->max_compute_work_group_count[0]);
                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &this->max_compute_work_group_count[1]);
                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &this->max_compute_work_group_count[2]);
                std::cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNTS (x, y, z): " << this->max_compute_work_group_count << std::endl;

                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &this->max_compute_work_group_size[0]);
                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &this->max_compute_work_group_size[1]);
                glGetInteger64i_v (GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &this->max_compute_work_group_size[2]);
                std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE (x, y, z): " << this->max_compute_work_group_size << std::endl;

                glGetIntegerv (GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &this->max_compute_shared_memory_size);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE: " << this->max_compute_shared_memory_size << " bytes" << std::endl;

                // Shader storage
                glGetIntegerv (GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &this->max_shader_storage_block_size);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE: " << this->max_shader_storage_block_size << std::endl;

                glGetIntegerv (GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &this->max_shader_storage_buffer_bindings);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: " << this->max_shader_storage_buffer_bindings << std::endl;

                // Combined
                glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS, &this->max_texture_image_units);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_TEXTURE_IMAGE_UNITS: " << this->max_texture_image_units << std::endl;

                glGetIntegerv (GL_MAX_TEXTURE_SIZE, &this->max_texture_size);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_TEXTURE_SIZE: " << this->max_texture_size << std::endl;

                glGetIntegerv (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &this->max_combined_texture_image_units);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " << this->max_combined_texture_image_units << std::endl;

                // Not mentioned on es3.1 Reference page https://registry.khronos.org/OpenGL-Refpages/es3.1/html/glGet.xhtml but can be queried:
                glGetIntegerv (GL_MAX_IMAGE_UNITS, &this->max_image_units);
                morph::gl::Util::checkError (__FILE__, __LINE__);
                std::cout << "GL_MAX_IMAGE_UNITS: " << this->max_image_units << std::endl;

                load_shaders();

                // No need to set any GL flags (though a derived class may need to if it is to render any graphics)

                morph::gl::Util::checkError (__FILE__, __LINE__);
            }

        public:
            // Add an implementation of load_shaders looking something like:
            //
            // void load_shaders() finals
            // {
            //   std::vector<morph::gl::ShaderInfo> shaders1 = {
            //     {GL_COMPUTE_SHADER, "Default.compute1.glsl", morph::gl::nonCompilingComputeShader, 0 }
            //   };
            //   this->my_compute_program_1.load_shaders (shaders1);
            //
            //   std::vector<morph::gl::ShaderInfo> shaders2 = {
            //     {GL_COMPUTE_SHADER, "Default.compute2.glsl", morph::gl::nonCompilingComputeShader, 0 }
            //   };
            //   this->my_compute_program_2.load_shaders (shaders2);
            // }
            //
            // Here "Default.compute.glsl" is the path to a file containing the GLSL
            // code. morph::defaultComputeShader is a const char* of some default GLSL code
            // text that will be used if the file cannot be accessed.
            //
            virtual void load_shaders() = 0;

        protected:
            //! The window (and OpenGL context) for this gl::compute_manager
            GLFWwindow* window = nullptr;

            //! Monitor info, obtained from glfw
            float monitor_xscale = 0.0f;
            float monitor_yscale = 0.0f;

            //! Desktop environment workarea info, obtained from glfw.
            int workarea_xpos=0;
            int workarea_ypos=0;
            int workarea_width=0;
            int workarea_height=0;

            //! Window size, if the derived class creates a Window
            morph::vec<int, 2> win_sz = { 640, 480 };
            //! The title for the object, if needed
            std::string title = "morph::gl_compute";

            // Various runtime-queryable limits on computation. The GL names for each of these can
            // be obtained by upper-casing and prepending 'GL_'. E.g.: max_compute_atomic_counters will
            // be set from a query of GL_MAX_COMPUTE_ATOMIC_COUNTERS.
            GLint max_compute_atomic_counters = -1;
            GLint max_compute_atomic_counters_buffers = -1;
            GLint max_compute_shader_storage_blocks = -1;
            GLint max_compute_texture_image_units = -1;
            GLint max_compute_uniform_blocks = -1;
            GLint max_compute_uniform_components = -1;
            GLint64 max_compute_work_group_invocations = -1;
            morph::vec<GLint64, 3> max_compute_work_group_count = {-1,-1,-1};
            morph::vec<GLint64, 3> max_compute_work_group_size = {-1,-1,-1};
            GLint max_compute_shared_memory_size = -1; // bytes
            GLint max_shader_storage_block_size = -1; // bytes?
            GLint max_shader_storage_buffer_bindings = -1;
            GLint max_texture_image_units = -1;
            GLint max_texture_size = -1; // bytes?
            GLint max_combined_texture_image_units = -1;
            GLint max_image_units = -1;

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

            //! An error callback function for the GLFW windowing library
            static void errorCallback (int error, const char* description)
            {
                std::cerr << "Error: " << description << " (code "  << error << ")\n";
            }

        private:
            static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
            {
                compute_manager<glver>* self = static_cast<compute_manager<glver>*>(glfwGetWindowUserPointer (_window));
                if (self->key_callback (key, scancode, action, mods)) {
                    std::cout << "key_callback returned\n";
                    self->compute();
                }
            }
            static void window_close_callback_dispatch (GLFWwindow* _window)
            {
                compute_manager<glver>* self = static_cast<compute_manager<glver>*>(glfwGetWindowUserPointer (_window));
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
            static constexpr bool debug_callback_extra = false;
            virtual void key_callback_extra (int key, int scancode, int action, int mods)
            {
                if constexpr (debug_callback_extra) {
                    std::cout << "Visual::key_callback_extra called for key=" << key << " scancode="
                              << scancode << " action=" << action << " and mods=" << mods << std::endl;
                }
            }

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

    } // namespace gl
} // namespace morph
