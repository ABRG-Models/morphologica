/****************************************************************************
**
** This is an example of how to build an opengl window from a QWindow. Adapted from the
** tutorial material from The Qt Company Ltd. with the following notice:
**
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
** * Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in
** the documentation and/or other materials provided with the
** distribution.
** * Neither the name of The Qt Company Ltd nor the names of its
** contributors may be used to endorse or promote products derived
** from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include <QtGui/QWindow>
#include <QOpenGLFunctions_4_1_Core>

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QOpenGLContext>

#include <iostream>

namespace morph {
    namespace qt {

        class OpenGLWindow : public QWindow, protected QOpenGLFunctions_4_1_Core
        {
            Q_OBJECT
        public:
            explicit OpenGLWindow (QWindow *parent = 0)
                : QWindow(parent)
                , m_update_pending(false)
                , m_animating(false)
                , m_context(0)
            {
                setSurfaceType (QWindow::OpenGLSurface);
            }

            ~OpenGLWindow() {}


            virtual void render (QPainter *painter) { Q_UNUSED (painter); }
            virtual void render()
            {
                // Somehow call back to Visual::render()
            }

            virtual void initialize() {} // no-op

            void setAnimating (bool animating)
            {
                m_animating = animating;
                if (animating) { renderLater(); }
            }

            void setContext() { m_context->makeCurrent(this); }

        public slots:
            void renderLater()
            {
                if (!m_update_pending) {
                    m_update_pending = true;
                    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
                }
            }

            void renderNow()
            {
                // std::cout << "OpenGLWindow::renderNow() called" << std::endl;
                if (!isExposed()) { return; }
                bool needsInitialize = false;
                if (!m_context) {
                    std::cout << "Creating new QOpenGLContext" << std::endl;
                    m_context = new QOpenGLContext(this);
                    m_context->setFormat(requestedFormat());
                    m_context->create();
                    needsInitialize = true;
                }
                m_context->makeCurrent(this);
                if (needsInitialize) {
                    // std::cout << "OpenGLWindow::renderNow() : NeedsInitialize" << std::endl;
                    initializeOpenGLFunctions();
                    initialize();
                }
                render();
                m_context->swapBuffers(this);
                if (m_animating) { renderLater(); }
            }

        protected:
            bool event (QEvent* event) override
            {
                switch (event->type()) {
                case QEvent::UpdateRequest:
                {
                    m_update_pending = false;
                    renderNow();
                    return true;
                }
                default: { return QWindow::event(event); }
                }
            }

            void exposeEvent (QExposeEvent* event) override
            {
                Q_UNUSED(event);
                if (isExposed()) { renderNow(); }
            }


        private:
            bool m_update_pending;
            bool m_animating;

            QOpenGLContext *m_context;
        };

    } // namespace qt
} // namespace morph
