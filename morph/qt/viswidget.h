#pragma once

#include <iostream>
#include <functional>

#include <QtWidgets/QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QWheelEvent>

// Visual is going to be owned by the QOpenGLWidget
#define OWNED_MODE 1
// Define morph::win_t before #including morph/Visual.h
namespace morph { using win_t = QOpenGLWidget; }
#include <morph/Visual.h>
// We need to be able to convert from Qt keycodes to morph keycodes
#include <morph/qt/keycodes.h>

namespace morph {
    namespace qt {

        // A morph::Visual widget
        struct viswidget : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core
        {
            // Unlike the GLFW or morph-in-a-QWindow schemes, we hold the morph::Visual
            // inside the widget.
            morph::Visual v;

            // In your Qt code, build VisualModels that should be added to the scene and add them to this.
            std::vector<std::unique_ptr<morph::VisualModel>> newvisualmodels;

            viswidget (QWidget* parent = 0) : QOpenGLWidget(parent)
            {
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
                // Make sure we can call gl functions
                initializeOpenGLFunctions();
                // Switch on multisampling anti-aliasing (with the num samples set in constructor)
                glEnable (GL_MULTISAMPLE);
                // Initialise morph::Visual
                v.init (this);
            }

            void resizeGL (int w, int h) override
            {
                v.set_winsize (w, h);
                this->update();
            }

            void paintGL() override
            {
                if (!this->newvisualmodels.empty()) {
                    // Now we iterate through newvisualmodels, finalize them and add them to morph::Visual
                    for (unsigned int i = 0; i < newvisualmodels.size(); ++i) {
                        this->newvisualmodels[i]->finalize();
                        this->v.addVisualModel (this->newvisualmodels[i]);
                    }
                    this->newvisualmodels.clear();
                }
                v.render();
            }

            void mousePressEvent (QMouseEvent* event)
            {
                v.set_cursorpos (event->x(), event->y());
                int bflg = event->button();
                int b = morph::mousebutton::unhandled;
                b = bflg & Qt::LeftButton ? morph::mousebutton::left : b;
                b = bflg & Qt::RightButton ? morph::mousebutton::right : b;
                int mflg = event->modifiers();
                int mods = 0;
                if (mflg & Qt::ControlModifier) { mods |= morph::keymod::CONTROL; }
                if (mflg & Qt::ShiftModifier) { mods |= morph::keymod::SHIFT; }
                v.mouse_button_callback (b, morph::keyaction::PRESS, mods);
                event->accept();
            }

            void mouseMoveEvent (QMouseEvent* event)
            {
                if (v.cursor_position_callback (event->x(), event->y())) {
                    this->update();
                }
                event->accept();
            }

            void mouseReleaseEvent (QMouseEvent* event)
            {
                v.set_cursorpos (event->x(), event->y());
                int bflg = event->button();
                int b =  morph::mousebutton::unhandled;
                b = bflg & Qt::LeftButton ? morph::mousebutton::left : b;
                b = bflg & Qt::RightButton ? morph::mousebutton::right : b;
                v.mouse_button_callback (b, morph::keyaction::RELEASE);
                event->accept();
            }

            void wheelEvent (QWheelEvent* event)
            {
                QPoint numSteps = event->angleDelta() / 120;
                v.scroll_callback (numSteps.x(), numSteps.y());
                this->update();
                event->accept();
            }

            // Keyboard events...
            void keyPressEvent (QKeyEvent* event)
            {
                int mflg = event->modifiers();
                int mods = 0;
                if (mflg & Qt::ControlModifier) { mods |= morph::keymod::CONTROL; }
                if (mflg & Qt::ShiftModifier) { mods |= morph::keymod::SHIFT; }
                int morph_keycode = morph::qt::qtkey_to_morphkey (event->key());
                // Could be keyaction::REPEAT in GLFW
                if (v.key_callback (morph_keycode, 0, morph::keyaction::PRESS, mods)) {
                    this->update();
                }
                event->accept();
            }
        };
    } // qt
} // morph
