/*
 * \file
 *
 * A testing visual class, following 01-triangles.
 *
 * \author Seb James
 * \date 2019
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include "GL3/gl3.h"
#endif

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

//enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { VertexBuffer, NormalBuffer, ColourBuffer, ElementBuffer, NumBuffers };
//enum Buffer_IDs { ArrayBuffer, NormalBuffer, ColourBuffer, NumBuffers };
// Attrib_IDs are the location indices in the GLSL file for the inputs.
enum Attrib_IDs { vPosition = 0, nPosition = 1, cPosition = 2 };
GLuint  vao;
GLuint  vbo[NumBuffers];
const GLuint  NumVertices = 6;

namespace morph {

    class TriangleVisual
    {
    public:
        GLuint shaderprog;

        //! Two triangles:

        GLuint  indices[6] = {0,1,2,3,4,5};

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

            glCreateVertexArrays (1, &vao);
            glBindVertexArray (vao);

            glCreateBuffers (NumBuffers, vbo);

            // Element buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[ElementBuffer]);
            cout << "sizeof(indices) is " << sizeof(indices) << endl;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Location buffer
            glBindBuffer (GL_ARRAY_BUFFER, vbo[VertexBuffer]);
            glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            // Say that in this vertex buffer object, which has
            // position in the GLSL of vPosition, there are 3 values
            // sized GL_FLOAT at a time (3D coords) they're of size
            // GL_FLOAT, they're not normalized, the stride is 0 and
            // we don't provide a pointer
            glVertexAttribPointer (vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            // Enable for this attribute at position vPosition.
            glEnableVertexAttribArray (vPosition);

            // Normal buffer
            glBindBuffer (GL_ARRAY_BUFFER, vbo[NormalBuffer]);
            glBufferData (GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
            glVertexAttribPointer (nPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (nPosition);

            // Colour buffer
            glBindBuffer (GL_ARRAY_BUFFER, vbo[ColourBuffer]);
            glBufferData (GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);
            glVertexAttribPointer (cPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (cPosition);

            glUseProgram (shaderprog);
        }

        void render() {
            //static const float colour[] = { 1.0f, 0.0f, 0.0f, 0.0f };
            //glClearBufferfv(GL_COLOR, 0, colour);
            glBindVertexArray(vao);
            //glDrawArrays( GL_TRIANGLES, 0, NumVertices );
            glDrawElements (GL_TRIANGLES, 6, VBO_ENUM_TYPE, 0);
            glBindVertexArray (0);
        }
    };

} // namespace
