/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation.
 *
 * This is the main visual scene class in morphologica and derives from morph::VisualOwnable, adding
 * window handling with GLFW3.
 *
 * This is the multiple context-safe version of VisualNoMX.
 *
 * It is further aliased to morph::Visual in morph/Visual.h.
 *
 * Created by Seb James on 2025/03/03
 *
 * \author Seb James
 * \date May 2025
 */
#pragma once

#ifndef _glfw3_h_ // glfw3 has not yet been externally included
# define GLFW_INCLUDE_NONE // Here, we tell GLFW that we will explicitly include GL3/gl3.h and GL/glext.h
# include <GLFW/glfw3.h>
#endif // _glfw3_h_

#include <mutex>
#include <chrono>

namespace morph {
    // With morph::Visual, we use a GLFW window which is owned by morph::Visual.
    using win_t = GLFWwindow;
}

#include <morph/VisualOwnableMX.h>
#include <morph/VisualGlfw.h>

namespace morph {

    /*!
     * Visual 'scene' class
     *
     * A class for visualising computational models on an OpenGL screen.
     *
     * Each VisualMX will have its own GLFW window and is essentially a "scene" containing a number
     * of objects. One object might be the visualisation of some data expressed over a
     * HexGrid. Another could be a GraphVisual object. The class handles mouse events to allow the
     * user to rotate and translate the scene, as well as use keys to generate particular
     * effects/views.
     *
     * It's possible to set the background colour of the scene (VisualMX::bgcolour), the location of
     * the objects in the scene (VisualMX::setSceneTransZ and friends) and the position and field of
     * view of the 'camera' (VisualMX::zNear, VisualMX::zFar and VisualMX::fov).
     *
     * \tparam glver The OpenGL version, encoded as a single int (see morph::gl::version)
     */
    template <int glver = morph::gl::version_4_1>
    class VisualMX : public morph::VisualOwnableMX<glver>
    {
    public:
        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So, this creates a
         * new window and a new OpenGL context.
         */
        VisualMX (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
        {
            this->window_w = _width;
            this->window_h = _height;
            this->title = _title;
            this->version_stdout = _version_stdout;

            this->init_resources();
            this->init_gl();

            // Special tasks: re-bind coordArrows and title text
            this->bindextra (this->coordArrows);
            this->bindextra (this->textModel);
        }

        //! Deconstructor destroys GLFW/Qt window and deregisters access to VisualResources
        virtual ~VisualMX()
        {
            this->setContext();
            glfwDestroyWindow (this->window);
            this->deconstructCommon();
        }

        // Do one-time init of the Visual's resources. This gets/creates the VisualResources,
        // registers this visual with resources, calls init_window for any glfw stuff that needs to
        // happen, and lastly initializes the freetype code.
        void init_resources()
        {
            morph::VisualGlfw<glver>::i().init(); // Init GLFW windows system
            // VisualResources provides font management. Ensure it exists in memory.
            morph::VisualResourcesMX<glver>::i().create();
            // Set up the window that will present the OpenGL graphics.  This has to
            // happen BEFORE the call to VisualResources::freetype_init()
            this->init_window();
            this->setContext(); // For freetype_init
            this->freetype_init();
            this->releaseContext();
        }

        void setSwapInterval() final
        {
            // Swap as fast as possible (fixes lag of scene with mouse movements)
            glfwSwapInterval (0);
        }

        //! Make this Visual the current one, so that when creating/adding a visual model, the vao
        //! ids relate to the correct OpenGL context.
        void setContext() final { glfwMakeContextCurrent (this->window); }

        //! swapBuffers implementation for glfw
        void swapBuffers() final { glfwSwapBuffers (this->window); }

        //! Lock the context to prevent accessing the OpenGL context from multiple threads
        //! then obtain the context.
        void lockContext()
        {
            this->context_mutex.lock();
            this->setContext();
        }

        //! Attempt to lock the context. If the mutex lock is obtained, set the OpenGL
        //! context and return true. If the mutex lock is not obtained, return false.
        bool tryLockContext()
        {
            if (this->context_mutex.try_lock()) {
                this->setContext();
                return true;
            }
            return false;
        }

        //! Release the OpenGL context and unlock the context mutex.
        void unlockContext()
        {
            this->releaseContext();
            this->context_mutex.unlock();
        }

        //! Release the OpenGL context
        void releaseContext() final { glfwMakeContextCurrent (nullptr); }

        /*!
         * \brief OpenGL context check
         *
         * You can see if the OpenGL context is held at any time in your program. This function
         * returns true if there is a non-null window and we currently 'have that context'. This
         * should return true after a call to Visual::setContext and false after a call to
         * Visual::releaseContext.
         */
        bool checkContext()
        {
            return this->window == nullptr ? false : (glfwGetCurrentContext() == this->window);
        }

        /*!
         * Set up the passed-in VisualModel (or indeed, VisualTextModel) with functions that need access to Visual attributes.
         */
        template <typename T>
        void bindmodel (std::unique_ptr<T>& model)
        {
            morph::VisualBase<glver>::template bindmodel<T> (model); // base class binds
            model->setContext = &morph::VisualBase<glver>::set_context;
            model->releaseContext = &morph::VisualBase<glver>::release_context;
            model->get_glfn = &morph::VisualOwnableMX<glver>::get_glfn;
        }

        template <typename T>
        void bindextra (std::unique_ptr<T>& model)
        {
            model->setContext = &morph::VisualBase<glver>::set_context;
            model->releaseContext = &morph::VisualBase<glver>::release_context;
            model->get_glfn = &morph::VisualOwnableMX<glver>::get_glfn;
        }

        /*
         * A note on setContext() in keepOpen/poll/waitevents/wait:
         *
         * I considered automatically calling setContext in these functions. However, the event
         * queue is not necessarily bound to the context (it depends on the platform), so I will
         * leave these as they are. The call to render() inside keepOpen() WILL correctly induce a
         * setContext() call.
         */

        /*!
         * Keep on rendering until readToFinish is set true. Used to keep a window open, and
         * responsive, while displaying the result of a simulation. FIXME: This won't work for two
         * or more windows because it will block.
         */
        void keepOpen()
        {
            while (this->readyToFinish == false) {
                glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
                this->render();
            }
        }

        /*!
         * Like keepOpen, but renders until paused is set false (or user signals they're ready to
         * finish), then returns.
         */
        void pauseOpen()
        {
            this->paused = true;
            while (this->paused == true && this->readyToFinish == false) {
                glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
                this->render();
            }
        }

        //! Wrapper around the glfw polling function
        void poll() { glfwPollEvents(); }
        //! A wait-for-events with a timeout wrapper
        void waitevents (const double& timeout) { glfwWaitEventsTimeout (timeout); }
        //! Collect events for timeout, returning after *all* the time elapsed
        void wait (const double& timeout)
        {
            using sc = std::chrono::steady_clock;
            sc::time_point t0 = sc::now();
            sc::time_point t1 = sc::now();
            int timeout_us = timeout * 1000000;
            while (std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() < timeout_us) {
                glfwWaitEventsTimeout (timeout/10.0);
                t1 = sc::now();
            }
        }

    private:

        void init_window()
        {
            this->window = glfwCreateWindow (this->window_w, this->window_h, this->title.c_str(), NULL, NULL);
            if (!this->window) {
                // Window or OpenGL context creation failed
                throw std::runtime_error("GLFW window creation failed!");
            }
            // now associate "this" object with mWindow object
            glfwSetWindowUserPointer (this->window, this);

            // Set up callbacks
            glfwSetKeyCallback (this->window, key_callback_dispatch);
            glfwSetMouseButtonCallback (this->window, mouse_button_callback_dispatch);
            glfwSetCursorPosCallback (this->window, cursor_position_callback_dispatch);
            glfwSetWindowSizeCallback (this->window, window_size_callback_dispatch);
            glfwSetWindowCloseCallback (this->window, window_close_callback_dispatch);
            glfwSetScrollCallback (this->window, scroll_callback_dispatch);

            glfwMakeContextCurrent (this->window);

            this->init_glad (glfwGetProcAddress);
        }

    private:
        //! Context mutex to prevent contexts being acquired in a non-threadsafe manner.
        std::mutex context_mutex;

        /*
         * GLFW callback dispatch functions
         */
        static void key_callback_dispatch (GLFWwindow* _window, int key, int scancode, int action, int mods)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->key_callback (key, scancode, action, mods)) {
                self->render();
            }
        }
        static void mouse_button_callback_dispatch (GLFWwindow* _window, int button, int action, int mods)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            self->mouse_button_callback (button, action, mods);
        }
        static void cursor_position_callback_dispatch (GLFWwindow* _window, double x, double y)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->cursor_position_callback (x, y)) {
                self->render();
            }
        }
        static void window_size_callback_dispatch (GLFWwindow* _window, int width, int height)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->window_size_callback (width, height)) {
                self->render();
            }
        }
        static void window_close_callback_dispatch (GLFWwindow* _window)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            self->window_close_callback();
        }
        static void scroll_callback_dispatch (GLFWwindow* _window, double xoffset, double yoffset)
        {
            VisualMX<glver>* self = static_cast<VisualMX<glver>*>(glfwGetWindowUserPointer (_window));
            if (self->scroll_callback (xoffset, yoffset)) {
                self->render();
            }
        }


    public:

        /*
         * Generic callback handlers
         */
        virtual bool key_callback (int _key, int scancode, int action, int mods)
        {
            return morph::VisualOwnableMX<glver>::template key_callback<true> (_key, scancode, action, mods);
        }
    };

} // namespace morph
