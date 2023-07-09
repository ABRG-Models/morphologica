/*
 * Qt compatibility for morphologica.
 *
 * This extends the morph::qt::OpenGLWindow base class to add the handling of mouse,
 * window resize and key-press events.
 *
 * Author: Seb James
 * Date: July 2023
 */

#pragma once

#include <functional>
#include <morph/qt/openglwindow.h>
#include <QVector2D>
#include <QMouseEvent>


namespace morph {

    class Visual;

    namespace qt {

        // An OpenGL enabled window that knows how to render via morphologica and can communicate events
        class qwindow : public morph::qt::OpenGLWindow
        {
        public:
            qwindow (morph::Visual* _v) : morph::qt::OpenGLWindow(nullptr) { this->v = _v;}
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
                // QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;
            }

            // Plus keyboard events...

            // The render event just calls back to Visual::render()
            void render() override { callback_render (this->v); }
            std::function<void(morph::Visual*)> callback_render;

        private:
            morph::Visual* v;
            QVector2D mousePressPosition;
        };

    } // namespace qt
} // namespace morph
