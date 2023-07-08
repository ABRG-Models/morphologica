/*
 * Qt compatibility for morphologica
 */

#pragma once

#include <morph/qt/openglwindow.h>

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
//#include <QtGui/QOpenGLBuffer>
//#include <QtGui/QOpenGLVertexArrayObject>

//#include <QMatrix4x4>
//#include <QQuaternion>
#include <QVector2D>
#include <QMouseEvent>
//#include <QBasicTimer>

#include <vector>

namespace morph {
    namespace qt {

        // An OpenGL enabled window that knows how to render via morphologica.
        class qwindow : public morph::qt::OpenGLWindow
        {
        public:
            qwindow() {}
            ~qwindow() {}

            // Gets called on a mouse press. This needs to call the callbacks...
            void mousePressEvent (QMouseEvent *e) override
            {
                // Save mouse press position. What's localPos() return type?
                mousePressPosition = QVector2D(e->localPos());
            }
            void mouseReleaseEvent (QMouseEvent *e) override
            {
                // Mouse release position - mouse press position
                QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;
            }

            // Plus keyboard events...

            void initialize() override
            {
                // Nothing to do? This is all init in Visual.
            }

            void render() override {}
            //void setPerspective (int w, int h); // may need to handle this? GLFW did that for me before.

        private:
            QVector2D mousePressPosition;
            // Visual* parent_vis; // urgh upcoming circular deps...
        };

    } // namespace qt
} // namespace morph
