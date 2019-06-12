#ifndef _VISUALBASE_H_
#define _VISUALBASE_H_

#include <GLFW/glfw3.h>

namespace morph {

    /*!
     * This is a small base class for Visual, which contains the
     * attribute @event_handling, which makes it possible for GLFW
     * event handling callbacks to access the actual instance of
     * Visual and its data members.
     */
    class VisualBase
    {
    protected:

        // Abstract callback functions
        //@{
        virtual void key_callback (GLFWwindow *window, int key, int scancode, int action, int mods) = 0;
        virtual void mouse_button_callback (GLFWwindow* win, int button, int action, int mods) = 0;
        virtual void cursor_position_callback (GLFWwindow* win, double x, double y) = 0;
        virtual void window_size_callback (GLFWwindow* win, int width, int height) = 0;
        //@}

        //! static pointer for event handling
        static VisualBase* event_handling;

        //! Set up event handling
        virtual void setEventHandling() { event_handling = this; }

        //! These dispatch callbacks to an instanced class derived from VisualBase
        //@{
        static void key_callback_dispatch (GLFWwindow* win, int key, int scancode, int action, int mods) {
            if (event_handling != (VisualBase*)0) {
                event_handling->key_callback (win, key, scancode, action, mods);
            }
        }
        static void mouse_button_callback_dispatch (GLFWwindow* win, int button, int action, int mods) {
            if (event_handling != (VisualBase*)0) {
                event_handling->mouse_button_callback (win, button, action, mods);
            }
        }
        static void cursor_position_callback_dispatch (GLFWwindow* win, double x, double y) {
            if (event_handling != (VisualBase*)0) {
                event_handling->cursor_position_callback (win, x, y);
            }
        }
        static void window_size_callback_dispatch (GLFWwindow* win, int width, int height) {
            if (event_handling != (VisualBase*)0) {
                event_handling->window_size_callback (win, width, height);
            }
        }
        //@}
    };
}

// Global static event_handling instance - will provide access to our Visual instance.
morph::VisualBase* morph::VisualBase::event_handling;

#endif // _VISUALBASE_H_
