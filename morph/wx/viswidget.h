#pragma once

#include <iostream>
#include <functional>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/gtk/glcanvas.h>

// Visual is going to be owned by the wxGLCanvas
#define OWNED_MODE 1
// Define morph::win_t before #including morph/Visual.h
namespace morph { using win_t = wxGLCanvas; }

#include <morph/Visual.h>
// We need to be able to convert from wxWidgets keycodes to morph keycodes
#include <morph/wx/keycodes.h>

namespace morph {
    namespace wx {
        // A morph::Visual widget
        struct viswidget : public wxGLCanvas
        {
            // In wx::viswidget, the morph::Visual is owned by the widget.
            morph::Visual<> v;

            std::unique_ptr<wxGLContext> m_context;

            // In your wx code, build VisualModels that should be added to the scene and add them to this.
            std::vector<std::unique_ptr<morph::VisualModel<>>> newvisualmodels;
            std::vector<morph::VisualModel<>*> model_ptrs;

            // if >-1, then that model needs a reinit.
            int needs_reinit = -1;
            void set_model_needs_reinit (int model_idx, bool reinit_required = true)
            {
                this->needs_reinit = reinit_required ? model_idx : -1;
            }

            viswidget (wxWindow* parent, const wxGLAttributes& canvasAttrs)
                : wxGLCanvas(parent, canvasAttrs)
            {

                // Explicitly create a new rendering context instance for this canvas.
                wxGLContextAttrs ctxAttrs;
                ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(3, 2).EndList();
                m_context = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);

                SetCurrent(*m_context);

                Bind(wxEVT_MOTION, &viswidget::OnMouseMove, this);
                Bind(wxEVT_LEFT_DOWN, &viswidget::OnMousePress, this);
                Bind(wxEVT_RIGHT_DOWN, &viswidget::OnMousePress, this);
                Bind(wxEVT_LEFT_UP, &viswidget::OnMouseRelease, this);
                Bind(wxEVT_RIGHT_UP, &viswidget::OnMouseRelease, this);
                Bind(wxEVT_MOUSEWHEEL, &viswidget::OnMouseWheel, this);
                Bind(wxEVT_KEY_DOWN, &viswidget::OnKeyPress, this);

                Bind(wxEVT_SIZE, &viswidget::OnSize, this);

                Bind(wxEVT_PAINT, &viswidget::OnPaint, this);

                initializeGL();
            }

        protected:

            void initializeGL()
            {
                // Switch on multisampling anti-aliasing (with the num samples set in constructor)
                glEnable (GL_MULTISAMPLE);
                // Initialise morph::Visual
                v.init (this);
            }

            void OnSize (wxSizeEvent& event)
            {
                event.Skip();
                const wxSize size = event.GetSize() * GetContentScaleFactor();
                int w = size.x;
                int h = size.y;
                v.set_winsize (w, h);
                Refresh (false);
            }

            void OnPaint (wxPaintEvent& WXUNUSED(event))
            {
                // This is a dummy, to avoid an endless succession of paint messages.
                // OnPaint handlers must always create a wxPaintDC.
                wxPaintDC dc(this);

                // This should not be needed, while we have only one canvas
                // SetCurrent(*m_GLContext);

                if (!this->newvisualmodels.empty()) {
                    // Now we iterate through newvisualmodels, finalize them and add them to morph::Visual
                    for (unsigned int i = 0; i < newvisualmodels.size(); ++i) {
                        this->newvisualmodels[i]->finalize();
                        this->model_ptrs.push_back (this->v.addVisualModel (this->newvisualmodels[i]));
                    }
                    this->newvisualmodels.clear();
                }
                if (this->needs_reinit > -1) {
                    this->model_ptrs[this->needs_reinit]->reinit();
                    this->needs_reinit = -1;
                }

                v.render();
                SwapBuffers();
            }

            void OnMousePress (wxMouseEvent& event)
            {
                event.Skip();
                wxPoint pos = event.GetPosition();
                int x = pos.x;
                int y = pos.y;
                v.set_cursorpos (x, y);
                int bflg = event.GetButton();
                int b = morph::mousebutton::unhandled;
                b = bflg & wxMOUSE_BTN_LEFT ? morph::mousebutton::left : b;
                b = bflg & wxMOUSE_BTN_RIGHT ? morph::mousebutton::right : b;
                int mflg = event.GetModifiers();
                int mods = 0;
                if (mflg & wxMOD_CONTROL) { mods |= morph::keymod::CONTROL; }
                if (mflg & wxMOD_SHIFT) { mods |= morph::keymod::SHIFT; }
                v.mouse_button_callback (b, morph::keyaction::PRESS, mods);
                event.Skip();
            }

            void OnMouseMove (wxMouseEvent& event)
            {
                wxPoint pos = event.GetPosition();
                int x = pos.x;
                int y = pos.y;
                if (v.cursor_position_callback (x,y)) { Refresh (false); }
                event.Skip();
            }

            void OnMouseRelease (wxMouseEvent& event)
            {
                event.Skip();
                wxPoint pos = event.GetPosition();
                int x = pos.x;
                int y = pos.y;
                v.set_cursorpos(x, y);
                int bflg = event.GetButton();
                int b = morph::mousebutton::unhandled;
                b = bflg & wxMOUSE_BTN_LEFT ? morph::mousebutton::left : b;
                b = bflg & wxMOUSE_BTN_RIGHT ? morph::mousebutton::right : b;
                v.mouse_button_callback(b, morph::keyaction::RELEASE);
            }

            void OnMouseWheel (wxMouseEvent& event)
            {
                int direction = event.GetWheelRotation()/120; // 1 or -1
                wxPoint numSteps;
                numSteps.x = 0;
                numSteps.y = direction;
                v.scroll_callback (numSteps.x, numSteps.y);
                Refresh (false);
                event.Skip();
            }

            void OnKeyPress (wxKeyEvent & event)
            {
                int mflg = event.GetModifiers();
                int mods = 0;
                if (mflg & wxMOD_CONTROL) {
                    mods |= morph::keymod::CONTROL;
                }
                if (mflg & wxMOD_SHIFT) {
                    mods |= morph::keymod::SHIFT;
                }
                int morph_keycode = morph::wx::wxkey_to_morphkey(event.GetKeyCode());
                // Could be keyaction::REPEAT in GLFW
                if (v.key_callback (morph_keycode, 0, morph::keyaction::PRESS, mods)) {
                    Refresh (false);
                }
                event.Skip();

            }
        };
    } // wx
} // morph
