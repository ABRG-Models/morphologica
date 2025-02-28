#pragma once

#include <iostream>
#include <functional>

struct QOpenGLWidget; // fwd decl

// Visual is going to be owned by the QOpenGLWidget
#define OWNED_MODE 1
// Define morph::win_t before #including morph/Visual.h
namespace morph { using win_t = QOpenGLWidget; }
#include <morph/Visual.h>

#include <QtWidgets/QOpenGLWidget>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QWheelEvent>

// We need to be able to convert from Qt keycodes to morph keycodes
#include <morph/qt/keycodes.h>


namespace morph {
    namespace qt {

        constexpr int gl_version = morph::gl::version_4_1;

        // How many separate OpenGL contexts (i.e. how many viswidget_mxs) to support in one Qt program?
        constexpr int max_contexts = 32; // with 32 we use 32 * 8 bytes of memory = 256 bytes.

        // A container class to manage a getProcAddress function from each viswidget_mx/QOpenGLWidget context
        struct gl_contexts
        {
            static auto& i() // The instance public function.
            {
                static gl_contexts instance;
                return instance;
            }

            // Set the context. Store the QOpenGLContext pointer into ctx_ptrs[widget_index].
            template<int widget_index>
            void set_context (QOpenGLContext* _ctx)
            {
                static_assert (widget_index < morph::qt::max_contexts);
                ctx_ptrs[widget_index] = _ctx;
            }

            // The static getProcAddress function for the index widget_index.
            template<int widget_index>
            static QFunctionPointer getProcAddress (const char* name)
            {
                static_assert (widget_index < morph::qt::max_contexts);
                if (morph::qt::gl_contexts::i().ctx_ptrs[widget_index] == nullptr) { return nullptr; }
                return morph::qt::gl_contexts::i().ctx_ptrs[widget_index]->getProcAddress (name);
            }

        private:
            gl_contexts() { ctx_ptrs = { nullptr }; }
            ~gl_contexts() {}
            std::array<QOpenGLContext*, morph::qt::max_contexts> ctx_ptrs;
        };

        // A morph::Visual widget. You have to choose and provide a widget_index in the range [0,
        // morph::gl::max_contexts)
        template<int widget_index>
        struct viswidget_mx : public QOpenGLWidget //, protected QOpenGLFunctions_4_1_Core
        {
            // Unlike the GLFW or morph-in-a-QWindow schemes, we hold the morph::Visual
            // inside the widget.
            morph::Visual<gl_version> v;

            // In your Qt code, build VisualModels that should be added to the scene and add them to this.
            std::vector<std::unique_ptr<morph::VisualModel<gl_version>>> newvisualmodels;
            std::vector<morph::VisualModel<gl_version>*> model_ptrs;

            // if >-1, then that model needs a reinit.
            int needs_reinit = -1;
            void set_model_needs_reinit (int model_idx, bool reinit_required = true)
            {
                this->needs_reinit = reinit_required ? model_idx : -1;
            }

            viswidget_mx (QWidget* parent = 0) : QOpenGLWidget(parent)
            {
                static_assert (widget_index < morph::qt::max_contexts);

                std::cout << "viswidget_mx\n";

                // You have to set the format in the constructor
                QSurfaceFormat format;
                format.setDepthBufferSize (4);
                format.setSamples (4);
                format.setStencilBufferSize (8);
                format.setVersion (4, 1);
                format.setProfile (QSurfaceFormat::CoreProfile);
                this->setFormat (format);
                this->setUpdateBehavior (QOpenGLWidget::NoPartialUpdate);
                this->setFocusPolicy (Qt::StrongFocus); // ensure keyPressEvents reach us
            }

        protected:

            void initializeGL() override
            {
                // Initialise morph::Visual, which must set up GLAD's access to the OpenGL context
                morph::qt::gl_contexts::i().set_context<widget_index> (this->context());
                v.init_glad (morph::qt::gl_contexts::getProcAddress<widget_index>);
                v.init (this);

                // Switch on multisampling anti-aliasing (with the num samples set in constructor)
                v.glfn->Enable (GL_MULTISAMPLE);
            }

            void resizeGL (int w, int h) override
            {
                double dpr = this->devicePixelRatio();
                v.set_winsize (static_cast<int>(std::round(w * dpr)), static_cast<int>(std::round(h * dpr)));
                this->update();
            }

            void paintGL() override
            {
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
            }

            void mousePressEvent (QMouseEvent* event) override
            {
                v.set_cursorpos (event->x(), event->y());
                int bflg = event->button();
                int b = morph::mousebutton::unhandled;
                b = bflg & Qt::LeftButton ? morph::mousebutton::left : b;
                b = bflg & Qt::RightButton ? morph::mousebutton::right : b;
                int mflg = event->modifiers();
                int mods = 0;
                if (mflg & Qt::ControlModifier) { mods |= morph::keymod::control; }
                if (mflg & Qt::ShiftModifier) { mods |= morph::keymod::shift; }
                v.mouse_button_callback (b, morph::keyaction::press, mods);
                event->accept();
            }

            void mouseMoveEvent (QMouseEvent* event) override
            {
                if (v.cursor_position_callback (event->x(), event->y())) {
                    this->update();
                }
                event->accept();
            }

            void mouseReleaseEvent (QMouseEvent* event) override
            {
                v.set_cursorpos (event->x(), event->y());
                int bflg = event->button();
                int b =  morph::mousebutton::unhandled;
                b = bflg & Qt::LeftButton ? morph::mousebutton::left : b;
                b = bflg & Qt::RightButton ? morph::mousebutton::right : b;
                v.mouse_button_callback (b, morph::keyaction::release);
                event->accept();
            }

            void wheelEvent (QWheelEvent* event) override
            {
                QPoint numSteps = event->angleDelta() / 120;
                v.scroll_callback (numSteps.x(), numSteps.y());
                this->update();
                event->accept();
            }

            // Keyboard events...
            void keyPressEvent (QKeyEvent* event) override
            {
                int mflg = event->modifiers();
                int mods = 0;
                if (mflg & Qt::ControlModifier) { mods |= morph::keymod::control; }
                if (mflg & Qt::ShiftModifier) { mods |= morph::keymod::shift; }
                int morph_keycode = morph::qt::qtkey_to_morphkey (event->key());
                // Could be keyaction::REPEAT in GLFW
                if (v.key_callback (morph_keycode, 0, morph::keyaction::press, mods)) {
                    this->update();
                }
                event->accept();
            }
        };
    } // qt
} // morph
