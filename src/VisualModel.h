#ifndef _VISUALMODEL_H_
#define _VISUALMODEL_H_

#include "GL3/gl3.h"
#include "GL/glext.h"

#include "tools.h"

#include "TransformMatrix.h"
using morph::TransformMatrix;

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

typedef GLuint VBOint;
#define VBO_ENUM_TYPE GL_UNSIGNED_INT

namespace morph {

    //! Forward declaration of a Visual class
    class Visual;

    //! The locations for the position, normal and colour vertex attributes in the GLSL program
    enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2 };

    //! This class is a base 'model' class. It has the common code to create the
    //! vertices for some individual model to be rendered in a 3-D scene.
    class VisualModel
    {
    public:
        VisualModel () {
            this->offset = {0.0, 0.0, 0.0};
        }
        VisualModel (GLuint sp, const array<float, 3> _offset) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);

            // In derived constructor: Do the computations to initialize the vertices
            // that will represent the model
            // this->initializeVertices();

            // Then common code for postVertexInit:
            // this->postVertexInit();

            // Here's how to unbind the VAO. Is that necessary? Seems not
            // glBindVertexArray(0);
        }

        virtual ~VisualModel() {
            // destroy buffers
            glDeleteBuffers (4, vbos);
            delete (this->vbos);
        }

        //! Common code to call after the vertices have been set up.
        void postVertexInit (void) {
            // Create vertex array object
            glCreateVertexArrays (1, &this->vao);
            glBindVertexArray (this->vao);

            // Allocate/create the vertex buffer objects
            this->vbos = new GLuint[numVBO];
            glCreateBuffers (numVBO, this->vbos);

            // Set up the indices buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);

            // Possibly release (unbind) the vertex buffers (but not index buffer)
            // Possible glVertexAttribPointer and glEnableVertexAttribArray?
            glUseProgram (this->shaderprog);
        }

        //! Initialize vertex buffer objects and vertex array object.
        virtual void initializeVertices (void) = 0;

        //! Render the VisualModel
        void render (void) {
            glBindVertexArray (this->vao);
            glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
            glBindVertexArray(0);
        }

        //! The model-specific view matrix.
        TransformMatrix<float> viewmatrix;

        //! Setter for offset, also updates viewmatrix.
        void setOffset (const array<float, 3>& _offset) {
            this->offset = _offset;
            this->viewmatrix.setToIdentity();
            this->viewmatrix.translate (this->offset);
        }

        //! Shift the offset, also updates viewmatrix.
        void shiftOffset (const array<float, 3>& _offset) {
            for (unsigned int i = 0; i < 3; ++i) {
                this->offset[i] += _offset[i];
            }
            this->viewmatrix.translate (this->offset);
        }

    protected:

        //! The offset of this Coordarrows. Note that this is not incorporated into
        //! the computation of the vertices, but is instead applied when the object is
        //! rendered as part of the model->world transformation.
        array<float, 3> offset;

        //! This enum contains the positions within the vbo array of the different vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;

        //! A copy of the reference to the shader program
        GLuint shaderprog;

        /*!
         * Compute positions and colours of vertices for the hexes and
         * store in these:
         */
        //@{
        //! The OpenGL Vertex Array Object
        GLuint vao;

        //! Vertex Buffer Objects stored in an array
        GLuint* vbos;

        //! CPU-side data for indices
        vector<VBOint> indices;
        //! CPU-side data for vertex positions
        vector<float> vertexPositions;
        //! CPU-side data for vertex normals
        vector<float> vertexNormals;
        //! CPU-side data for vertex colours
        vector<float> vertexColors;
        //@}

        //! I guess we'll need a shader program.
        GLuint* shaderProgram;

        //! Push three floats onto the vector of floats @vp
        //@{
        void vertex_push (const float& x, const float& y, const float& z, vector<float>& vp) {
            vp.push_back (x);
            vp.push_back (y);
            vp.push_back (z);
        }
        void vertex_push (const array<float, 3>& arr, vector<float>& vp) {
            vp.push_back (arr[0]);
            vp.push_back (arr[1]);
            vp.push_back (arr[2]);
        }
        void vertex_push (const Vector3<float>& vec, vector<float>& vp) {
            vp.push_back (vec.x);
            vp.push_back (vec.y);
            vp.push_back (vec.z);
        }
        //@}

        //! Set up a vertex buffer object
        void setupVBO (GLuint& buf, vector<float>& dat, unsigned int bufferAttribPosition) {
            int sz = dat.size() * sizeof(float);
            glBindBuffer (GL_ARRAY_BUFFER, buf);
            glBufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            glVertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            glEnableVertexAttribArray (bufferAttribPosition);
        }

    };

} // namespace morph

#endif // _VISUALMODEL_H_
