#pragma once

#include <QtWidgets/QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

#define USING_MORPHWIDGET 1
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
                std::cout << "viswidget initializeGL\n";
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

            void resizeGL (int w, int h) override
            {
                std::cout << "viswidget resizeGL\n";
                v.set_winsize (w, h);
            }

            void paintGL() override
            {
                std::cout << "viswidget paintGL\n";
                makeCurrent();
                v.render();
            }
        };
    } // qt
} // morph
