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
        // Abstract key_callback function
        virtual void key_callback (GLFWwindow *window, int key, int scancode, int action, int mods) = 0;

        static VisualBase* event_handling;

        virtual void setEventHandling() { event_handling = this; }

        //! Dispatches callbacks to a instanced class derived from VisualBase
        static void key_callback_dispatch (GLFWwindow* window, int key, int scancode, int action, int mods) {
            if (event_handling != (VisualBase*)0) {
                event_handling->key_callback (window, key, scancode, action, mods);
            }
        }
    };
}

// Global static event_handling instance - will provide access to our Visual instance.
morph::VisualBase* morph::VisualBase::event_handling;

#endif // _VISUALBASE_H_
