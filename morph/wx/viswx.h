/*
 * Defines morph::wx::Canvas and morph::wx::Frame. These two classes are extensions of
 * wxWidgets classes that allow for the use of morph::Visual (which is owned by
 * morph::wx::Canvas) to draw with OpenGL in a wxWidgets program.
 */
#include <memory>
#include <vector>
#include <sstream>
#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>
#include <wx/colordlg.h>

// Visual is going to be owned either by the morph::wx::canvas or by the morph::wx::frame
#define OWNED_MODE 1
// Define morph::win_t before #including morph/Visual.h
namespace morph { using win_t = wxGLCanvas; }

#include <morph/gl/version.h>
#include <morph/Visual.h>
// We need to be able to convert from wxWidgets keycodes to morph keycodes
#include <morph/wx/keycodes.h>

namespace morph {
    namespace wx {

        // This is the OpenGL version you will attempt to create a context for. Example version:
        // morph::gl::version_4_1
        template<int glver>
        class Canvas : public wxGLCanvas
        {
        public:
            Canvas (wxFrame* parent, const wxGLAttributes& canvasAttrs)
                : wxGLCanvas(parent, canvasAttrs)
            {
                wxGLContextAttrs ctxAttrs;
                ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(morph::gl::version::major(glver),
                                                                     morph::gl::version::minor(glver)).EndList();
                this->glContext = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);

                if (!this->glContext->IsOK()) {
                    std::stringstream ee;
                    ee << "This sample needs an OpenGL " << morph::gl::version::vstring(glver) << " capable driver.";
                    wxMessageBox (ee.str(), "OpenGL version error", wxOK | wxICON_INFORMATION, this);
                }

                // Bind events to functions in the Canvas constructor
                Bind (wxEVT_PAINT, &morph::wx::Canvas<glver>::OnPaint, this);
                Bind (wxEVT_SIZE, &morph::wx::Canvas<glver>::OnSize, this);
                Bind (wxEVT_MOTION, &morph::wx::Canvas<glver>::OnMouseMove, this);
                Bind (wxEVT_LEFT_DOWN, &morph::wx::Canvas<glver>::OnMousePress, this);
                Bind (wxEVT_RIGHT_DOWN, &morph::wx::Canvas<glver>::OnMousePress, this);
                Bind (wxEVT_LEFT_UP, &morph::wx::Canvas<glver>::OnMouseRelease, this);
                Bind (wxEVT_RIGHT_UP, &morph::wx::Canvas<glver>::OnMouseRelease, this);
                Bind (wxEVT_MOUSEWHEEL, &morph::wx::Canvas<glver>::OnMouseWheel, this);
                Bind (wxEVT_KEY_DOWN, &morph::wx::Canvas<glver>::OnKeyPress, this);
            }

            bool InitializeOpenGLFunctions()
            {
                GLenum err = glewInit();
                if (GLEW_OK != err) {
                    wxLogError("OpenGL GLEW initialization failed: %s",
                               reinterpret_cast<const char *>(glewGetErrorString(err)));
                    return false;
                }
                wxLogDebug("Status: Using GLEW %s", reinterpret_cast<const char *>(glewGetString(GLEW_VERSION)));
                return true;
            }

            bool InitializeOpenGL()
            {
                if (!this->glContext) { return false; }
                SetCurrent (*this->glContext.get());
                if (!InitializeOpenGLFunctions()) {
                    wxMessageBox("Error: Could not initialize OpenGL function pointers.",
                                 "OpenGL initialization error", wxOK | wxICON_INFORMATION, this);
                    return false;
                }
                // Switch on multisampling anti-aliasing (with the num samples set in constructor)
                glEnable (GL_MULTISAMPLE);
                // Initialise morph::Visual
                v.init (this);
                this->glInitialized = true;
                return this->glInitialized;
            }

            // Check if any of the VisualModels in the Visual need a reinit and then call Visual::render
            void OnPaint (wxPaintEvent &event)
            {
                wxPaintDC dc(this);
                if (!this->glInitialized) { return; }
                SetCurrent(*this->glContext.get());
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

            void OnSize (wxSizeEvent &event)
            {
                bool firstApperance = IsShownOnScreen() && !this->glInitialized;
                if (firstApperance) {
                    InitializeOpenGL();
                    if (this->glInitialized) {
                        this->setupVisualModels();
                    }
                }
                if (this->glInitialized) {
                    const wxSize size = event.GetSize() * GetContentScaleFactor();
                    v.set_winsize (static_cast<int>(size.x), static_cast<int>(size.y));
                }
                // Refresh(false); // maybe
                event.Skip();
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
                b = bflg == wxMOUSE_BTN_LEFT ? morph::mousebutton::left : b;
                b = bflg == wxMOUSE_BTN_RIGHT ? morph::mousebutton::right : b;
                int mflg = event.GetModifiers();
                int mods = 0;
                if (mflg & wxMOD_CONTROL) { mods |= morph::keymod::control; }
                if (mflg & wxMOD_SHIFT) { mods |= morph::keymod::shift; }
                v.mouse_button_callback (b, morph::keyaction::press, mods);
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
                v.mouse_button_callback(b, morph::keyaction::release);
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
                    mods |= morph::keymod::control;
                }
                if (mflg & wxMOD_SHIFT) {
                    mods |= morph::keymod::shift;
                }
                int morph_keycode = morph::wx::wxkey_to_morphkey(event.GetKeyCode());
                // Could be keyaction::REPEAT in GLFW
                if (v.key_callback (morph_keycode, 0, morph::keyaction::press, mods)) {
                    Refresh (false);
                }
                event.Skip();
            }

            // A funtion to be extended with morph::VisualModel setup.
            virtual void setupVisualModels()
            {
            }

            // API for user to say that model 4 (say) need to be reinitialized.
            void set_model_needs_reinit (int model_idx, bool reinit_required = true)
            {
                this->needs_reinit = reinit_required ? model_idx : -1;
            }

            // In your wx code, build VisualModels that should be added to the scene and add them to this.
            std::vector<std::unique_ptr<morph::VisualModel<glver>>> newvisualmodels;
            std::vector<morph::VisualModel<glver>*> model_ptrs;
            // if >-1, then that model needs a reinit.
            int needs_reinit = -1;

            bool ready() { return this->glInitialized; }

            morph::Visual<glver> v;

        private:
            std::unique_ptr<wxGLContext> glContext;
            bool glInitialized = false;
        };

        // morph::wx::Frame is to be extended. Note that a default GL version of 4.1 is given here.
        template <int glver = morph::gl::version_4_1>
        class Frame : public wxFrame
        {
        public:
            Frame(const wxString &title)
                : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
            {
                wxGLAttributes vAttrs;
                vAttrs.PlatformDefaults().Defaults().EndList();
                if (wxGLCanvas::IsDisplaySupported(vAttrs)) {
                    // canvas becomes a child of this wxFrame which is responsible for deallocation
                    this->canvas = new morph::wx::Canvas<glver>(this, vAttrs);
                    this->canvas->SetMinSize (FromDIP (wxSize(640, 480)));
                } else {
                    throw std::runtime_error ("wxGLCanvas::IsDisplaySupported(vAttrs) returned false");
                }
            }

        protected:
            // A morph::wx::Frame contains a morph::wx::Canvas
            morph::wx::Canvas<glver>* canvas;
        };

    } // wx
} // morph
