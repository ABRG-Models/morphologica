/*
 * A testing visual class, following 01-triangles.
 */

#ifndef _TRIANGLEVISUAL_H_
#define _TRIANGLEVISUAL_H_

#include "GL3/gl3.h"

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
//enum Buffer_IDs { ArrayBuffer, NormalBuffer, ColourBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };
GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];
const GLuint  NumVertices = 6;

namespace morph {

    class TriangleVisual
    {
    public:
        GLuint shaderprog;

        //! Two triangles:
        GLfloat  vertices[NumVertices][3] = {
            { -0.90f, -0.90f, 0.3f }, {  0.85f, -0.90f, 2.0f }, { -0.90f,  0.9f, 2.0f },
            {  0.90f, -0.85f, 0.3f }, {  0.90f,  0.90f, 0.3f }, { -0.85f,  0.90f, 0.3f }
        };
        GLfloat  normals[NumVertices][3] = {
            { -0.0f, -0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f },
            {  0.0f, -0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f }
        };
        GLfloat  colours[NumVertices][3] = {
            {  0.0f, 0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f },
            {  0.0f, 1.0f, 1.0f }, {  0.0f, 1.0f, 0.0f }, { -0.0f,  1.0f, 0.0f }
        };

        TriangleVisual(GLuint sp) {
            this->shaderprog = sp;
            glGenVertexArrays (NumVAOs, VAOs);
            glBindVertexArray (VAOs[Triangles]);

            glCreateBuffers (NumBuffers, Buffers);
            glBindBuffer (GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
            glBufferStorage (GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);
#if 0
            glBindBuffer (GL_ARRAY_BUFFER, Buffers[NormalBuffer]);
            glBufferStorage (GL_ARRAY_BUFFER, sizeof(normals), normals, 0);
            glBindBuffer (GL_ARRAY_BUFFER, Buffers[ColourBuffer]);
            glBufferStorage (GL_ARRAY_BUFFER, sizeof(colours), colours, 0);
#endif
            glUseProgram (shaderprog);

            glVertexAttribPointer (vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (vPosition);
        }

        void render (void) {
            //static const float colour[] = { 1.0f, 0.0f, 0.0f, 0.0f };
            //glClearBufferfv(GL_COLOR, 0, colour);
            glBindVertexArray( VAOs[Triangles] );
            glDrawArrays( GL_TRIANGLES, 0, NumVertices );
        }
    };

} // namespace

#endif // _TRIANGLEVISUAL_H_
