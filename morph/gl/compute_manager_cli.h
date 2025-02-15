/*!
 * \file
 *
 * OpenGL compute shading. This is a 'manager class' for compute shading that requires no window
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

// EGL and gbm for a headless OpenGL context
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>

#include <chrono>
#include <fcntl.h>

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
        struct compute_manager_cli
        {
            compute_manager_cli() { this->t0 = sc::now(); }
            ~compute_manager_cli()
            {
                // cleanup
            }

            //! Init GLFW and then the GLFW window. What if you want to set window width
            //! based on GLFW info such as this->workarea_width? In that case, instead
            //! of calling this do-it-all init function, just call the init functions separately:
            //! init_glfw(); do_stuff(); init_window(); init_gl();
            void init()
            {
                this->init_context();
                this->setContext();
                // Finally init GL
                this->init_gl();
            }

            void setContext()
            {
                bool res = eglMakeCurrent (this->egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, this->core_ctx);
                if (res == false) {
                    throw std::runtime_error ("Failed to eglMakeCurrent for headless GL");
                }
            }
            void releaseContext()
            {
            }

            // You will need to re-implement this function
            virtual void compute() = 0;

            //! Set to true when the program should end
            bool readyToFinish = false;

        protected:

            void init_context()
            {
                // Set up a GBM device
                int fd = open ("/dev/dri/renderD128", O_RDWR);
                if (fd <= 0) {
                    throw std::runtime_error ("Failed to open renderD128 device for headless GL");
                }
                struct gbm_device* gbm = gbm_create_device (fd);
                if (gbm == nullptr) {
                    throw std::runtime_error ("Failed to gbm_create_device for headless GL");
                }

                // setup EGL from the GBM device
                this->egl_dpy = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, NULL);
                if (this->egl_dpy == NULL) {
                    throw std::runtime_error ("Failed to eglGetPlatformDisplay for headless GL");
                }

                bool res = eglInitialize (this->egl_dpy, NULL, NULL);
                if (res == false) {
                    throw std::runtime_error ("Failed to eglInitialize display for headless GL");
                }

                const char* egl_extension_st = eglQueryString (egl_dpy, EGL_EXTENSIONS);
                if (strstr (egl_extension_st, "EGL_KHR_create_context") == NULL) {
                    throw std::runtime_error ("query response did not contain EGL_KHR_create_context");
                }
                if (strstr (egl_extension_st, "EGL_KHR_surfaceless_context") == NULL) {
                    throw std::runtime_error ("query response did not contain EGL_KHR_surfaceless_context");
                }
                static const EGLint config_attribs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                    EGL_NONE
                };
                EGLConfig cfg;
                EGLint count;

                res = eglChooseConfig (this->egl_dpy, config_attribs, &cfg, 1, &count);
                if (res == false) {
                    throw std::runtime_error ("Failed to eglChooseConfig for headless GL");
                }

                res = eglBindAPI (EGL_OPENGL_ES_API);
                if (res == false) {
                    throw std::runtime_error ("Failed to eglBindAPI for headless GL");
                }

                static const EGLint attribs[] = {
                    EGL_CONTEXT_MAJOR_VERSION, morph::gl::version::major(glver),
                    EGL_CONTEXT_MINOR_VERSION, morph::gl::version::minor(glver),
                    EGL_NONE
                };
                this->core_ctx = eglCreateContext (this->egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
                if (this->core_ctx == EGL_NO_CONTEXT) {
                    throw std::runtime_error ("Failed to eglCreateContext for headless GL");
                }
            }

            // Initialize OpenGL shaders, set any GL flags required
            void init_gl()
            {
                unsigned char* glv = (unsigned char*)glGetString(GL_VERSION);
                std::cout << "compute_manager_cli<" << morph::gl::version::vstring (glver)
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
            //! OpenGL context for this gl::compute_manager_cli goes here
            EGLDisplay egl_dpy;
            EGLContext core_ctx;

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
        };

    } // namespace gl
} // namespace morph
