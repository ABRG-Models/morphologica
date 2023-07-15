#pragma once

#include <iostream>

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

namespace morph {
    namespace qt {

        // A morph::Visual widget
        struct viswidget : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core
        {
            // Unlike the GLFW or morph-in-a-QWindow schemes, we hold the morph::Visual
            // inside the widget.
            morph::Visual v;

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

            void paintGL() override { v.render(); }

            void mousePressEvent (QMouseEvent* event)
            {
                v.set_cursorpos (event->x(), event->y());
                int bflg = event->button();
                int b =  morph::mousebutton::unhandled;
                b = bflg & Qt::LeftButton ? morph::mousebutton::left : b;
                b = bflg & Qt::RightButton ? morph::mousebutton::right : b;
                int mods = 0; // writeme
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
                int b = event->button() & Qt::LeftButton ? 1 : 0;
                v.mouse_button_callback (b, 0);
                event->accept();
            }

            void wheelEvent (QWheelEvent* event)
            {
                QPoint numSteps = event->angleDelta() / 120;
                v.scroll_callback (numSteps.x(), numSteps.y());
                this->update();
                event->accept();
            }
        };
    } // qt
} // morph
