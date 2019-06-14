/*
 * A testing visual class, following 01-triangles.
 */

#ifndef _TRIANGLEVISUAL_H_
#define _TRIANGLEVISUAL_H_

#include "GL3/gl3.h"

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

        GLuint loc_attrib = 0; // Vertex location attribute is in 0th location in glsl file
        GLfloat  vertices[NumVertices][3] = {
            { -0.90f, -0.90f, 0.3f }, {  0.85f, -0.90f, 2.0f }, { -0.90f,  0.9f, 2.0f },
            {  0.90f, -0.85f, 0.3f }, {  0.90f,  0.90f, 0.3f }, { -0.85f,  0.90f, 0.3f }
        };
        GLuint normal_attrib = 1;
        GLfloat  normals[NumVertices][3] = {
            { -0.0f, -0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f },
            {  0.0f, -0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f }
        };
        GLuint col_attrib = 2;
        GLfloat  colours[NumVertices][3] = {
            {  0.0f, 0.0f, 1.0f }, {  0.0f, 0.0f, 1.0f }, { -0.0f,  0.0f, 1.0f },
            {  0.0f, 1.0f, 1.0f }, {  0.0f, 1.0f, 0.0f }, { -0.0f,  1.0f, 0.0f }
        };

        TriangleVisual(GLuint sp) {
            this->shaderprog = sp;

            glCreateVertexArrays (1, &vao);
            glBindVertexArray (vao);


#if 0 // OpenGL 4.5/ARB_direct_state_access example calls:
            // Enable my attributes
            glEnableVertexArrayAttrib(array, loc_attrib);
            glEnableVertexArrayAttrib(array, normal_attrib);
            glEnableVertexArrayAttrib(array, col_attrib);
            // Set up the formats for my attributes
            glVertexArrayAttribFormat(array, loc_attrib,      3, GL_FLOAT, GL_FALSE, 0);
            glVertexArrayAttribFormat(array, normal_attrib,   3, GL_FLOAT, GL_FALSE, 12);
            glVertexArrayAttribFormat(array, col_attrib,      2, GL_FLOAT, GL_FALSE, 24);
            // Make my attributes all use binding 0
            glVertexArrayAttribBinding(array, loc_attrib,      0);
            glVertexArrayAttribBinding(array, normal_attrib,   0);
            glVertexArrayAttribBinding(array, col_attrib, 0);
#endif

            glCreateBuffers (NumBuffers, vbo);

            // Element buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[ElementBuffer]);
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
#if 0
        void setupVBO (GLuint& buf,
                       GLfloat**  dat,
                       const char* arrayname) {
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            int sz = (*this->data).size() * sizeof(float);
            glBufferData (GL_ARRAY_BUFFER, sz, dat, GL_STATIC_DRAW);
            // Something like:
            glVertexAttribPointer (vPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray (vPosition);
        }
#endif
        void render (void) {
            //static const float colour[] = { 1.0f, 0.0f, 0.0f, 0.0f };
            //glClearBufferfv(GL_COLOR, 0, colour);
            glBindVertexArray(vao);
            //glDrawArrays( GL_TRIANGLES, 0, NumVertices );
            glDrawElements (GL_TRIANGLES, 6, VBO_ENUM_TYPE, 0);
            glBindVertexArray (0);
        }
    };

} // namespace

#endif // _TRIANGLEVISUAL_H_
