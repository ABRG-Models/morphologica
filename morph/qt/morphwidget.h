#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace morph {
    namespace qt {

        struct morphwidget : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core
        {
            // Unlike the GLFW or morph-in-a-QWindow schemes, we hold the morph::Visual
            // inside the widget.
            morph::Visual v;

            morphwidget (QWidget* parent) : QOpenGLWidget(parent) {}
        protected:
            void initializeGL() override
            {
                QSurfaceFormat format;
                format.setDepthBufferSize(24);
                format.setStencilBufferSize(8);
                format.setVersion(3, 2);
                format.setProfile(QSurfaceFormat::CoreProfile);
                this->setFormat(format);

                initializeOpenGLFunctions();
            }

            void resizeGL(int w, int h) overrride
            {
            }

            void paintGL() override
            {
                // Visual painting...
            }
        };
    } // qt
} // morph
