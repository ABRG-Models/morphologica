#pragma once

#include <QtWidgets/QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QMouseEvent>

#define USING_MORPHWIDGET 1
namespace morph { using win_t = QOpenGLWidget; }
#include <morph/Visual.h>
#include <iostream>

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
                setUpdateBehavior (QOpenGLWidget::NoPartialUpdate);
            }

        protected:

            void initializeGL() override
            {
                QSurfaceFormat format;
                format.setDepthBufferSize (4);
                format.setSamples (24);
                format.setStencilBufferSize (8);
                format.setVersion (4, 1);
                format.setProfile (QSurfaceFormat::CoreProfile);
                this->setFormat (format);
                initializeOpenGLFunctions();
                v.init (this);
            }

            void resizeGL (int w, int h) override { v.set_winsize (w, h); }

            void paintGL() override { v.render(); }

            void mousePressEvent (QMouseEvent* event)
            {
                v.set_cursorpos (event->x(), event->y());
                int b = event->button() & Qt::LeftButton ? 1 : 0;
                v.mouse_button_callback (b, 1);
            }

            void mouseMoveEvent (QMouseEvent* event)
            {
                if (v.cursor_position_callback (event->x(), event->y())) {
                    this->update();
                }
            }

            void mouseReleaseEvent (QMouseEvent* event)
            {
                v.set_cursorpos (event->x(), event->y());
                int b = event->button() & Qt::LeftButton ? 1 : 0;
                v.mouse_button_callback (b, 0);
            }
        };
    } // qt
} // morph
