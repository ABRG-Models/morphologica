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
            //cout << "indices.size(): " << this->indices.size() << endl;
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

        //! The offset of this VisualModel. Note that this is not incorporated into
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

        /*!
         * Create a tube from start to end, with radius r.
         *
         * @idx The index into the 'vertex array'
         * @start The start of the tube
         * @end The end of the tube
         * @col The tube colour
         * @r Radius of the tube
         * @segments Number of segments used to render the tube
         */
        void computeTube (GLushort& idx, array<float, 3> start, array<float, 3> end,
                          array<float, 3> colStart, array<float, 3> colEnd,
                          float r = 1.0f, int segments = 12) {

            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.

            // The vector from start to end defines a vector and a plane. Find a 'circle' of points in that plane.
            Vector3<float> vstart (start);
            Vector3<float> vend (end);
            //cout << "Compute tube from " << vstart.asString() << "to " << vend.asString() << endl;
            Vector3<float> v = vend - vstart;
            v.renormalize();
            //cout << "Normal vector v is " << v.asString() << endl;

            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^
            // v. The unit in-plane vector is inplane.normalise. Can now use that
            // vector in the plan to define a point on the circle.
            Vector3<float> rand_vec;
            rand_vec.randomize();
            Vector3<float> inplane = rand_vec * v;
            inplane.renormalize();
            //cout << "in-plane vector is " << inplane.asString() << endl;

            // Now use parameterization of circle inplane = p1-x1 and
            // c1(t) = ( (p1-x1).normalized sin(t) + v.normalized cross (p1-x1).normalized * cos(t) )
            // c1(t) = ( inplane sin(t) + v * inplane * cos(t)
            Vector3<float> v_x_inplane = v * inplane;
            //cout << "v ^ inplane vector is " << v_x_inplane.asString() << endl;
            // Point on circle: Vector3<float> c = inplane * sin(t) + v_x_inplane * cos(t);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            //cout << "Central point of vstart cap is " << vstart.asString() << endl;
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                //cout << "t is " << t << endl;
                Vector3<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                //cout << "point on vstart cap is " << (vstart+c).asString() << endl;
                this->vertex_push (-v, this->vertexNormals); // -v
                this->vertex_push (colStart, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = (float)j * morph::TWO_PI_F/(float)segments;
                //cout << "t is " << t << endl;
                Vector3<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                //cout << "point on vend cap is " << (vend+c).asString() << endl;
                this->vertex_push (v, this->vertexNormals); // +v
                this->vertex_push (colEnd, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            //cout << "vend cap is " << vend.asString() << endl;
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            // Note: number of vertices = segments * 2 + 2.
            int nverts = (segments * 2) + 2;

            // After creating vertices, push all the indices.
            GLushort capMiddle = idx;
            GLushort capStartIdx = idx + 1;
            GLushort endMiddle = idx + (GLushort)nverts - 1;
            GLushort endStartIdx = capStartIdx + segments;

            //cout << "start cap" << endl;
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                //cout << "add " << capMiddle << " to indices\n";
                this->indices.push_back (capStartIdx + j);
                //cout << "add " << (capStartIdx+j) << " to indices\n";
                this->indices.push_back (capStartIdx + 1 + j);
                //cout << "add " << (capStartIdx+1+j) << " to indices\n";
            }
            // Last one
            this->indices.push_back (capMiddle);
            //cout << "add " << capMiddle << " to indices\n";
            this->indices.push_back (capStartIdx + segments - 1);
            //cout << "add " << (capStartIdx + segments - 1) << " to indices\n";
            this->indices.push_back (capStartIdx);
            //cout << "add " << (capStartIdx) << " to indices\n";

            //cout << "sides" << endl;
            for (int j = 0; j < segments; j++) {
                // Two triangles per side; 1:
                this->indices.push_back (capStartIdx + j);
                //cout << "1. add " << (capStartIdx + j) << " to indices\n";
                if (j == (segments-1)) {
                    this->indices.push_back (capStartIdx);
                    //cout << "1. add " << (capStartIdx) << " to indices\n";
                } else {
                    this->indices.push_back (capStartIdx + 1 + j);
                    //cout << "1. add " << (capStartIdx + j + 1) << " to indices\n";
                }
                this->indices.push_back (endStartIdx + j);
                //cout << "1. add " << (endStartIdx + j) << " to indices\n";
                // 2:
                this->indices.push_back (endStartIdx + j);
                //cout << "2. add " << (endStartIdx + j) << " to indices\n";
                if (j == (segments-1)) {
                    this->indices.push_back (endStartIdx);
                    //cout << "2. add " << (endStartIdx) << " to indices\n";
                } else {
                    this->indices.push_back (endStartIdx + 1 + j);
                    //cout << "2. add " << (endStartIdx + 1 + j) << " to indices\n";
                }
                if (j == (segments-1)) {
                    this->indices.push_back (capStartIdx);
                    //cout << "2. add " << (capStartIdx) << " to indices\n";
                } else {
                    this->indices.push_back (capStartIdx + j + 1);
                    //cout << "2. add " << (capStartIdx + j + 1) << " to indices\n";
                }
            }

            // bottom cap
            //cout << "vend cap" << endl;
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                //cout << "add " << (endMiddle) << " to indices\n";
                this->indices.push_back (endStartIdx + j);
                //cout << "add " << (endStartIdx + j) << " to indices\n";
                this->indices.push_back (endStartIdx + 1 + j);
                //cout << "add " << (endStartIdx + 1 + j) << " to indices\n---\n";
            }
            // Last one
            this->indices.push_back (endMiddle);
            //cout << "add " << (endMiddle) << " to indices\n";
            this->indices.push_back (endStartIdx + segments - 1);
            //cout << "add " << (endStartIdx - 1 + segments) << " to indices\n";
            this->indices.push_back (endStartIdx);
            //cout << "add " << (endStartIdx) << " to indices\n";

            // Update idx
            idx += nverts;
        }

        /*!
         * Code for creating a sphere as part of this model. I'll use a sphere at the centre of the arrows.
         *
         * @idx The index into the 'vertex indices array'
         * @so The sphere offset. Where to place this sphere...
         * @sc The sphere colour.
         * @r Radius of the sphere
         * @rings Number of rings used to render the sphere
         * @segments Number of segments used to render the sphere
         */
        void computeSphere (GLushort& idx, array<float, 3> so, array<float, 3> sc, float r = 1.0f,
                            int rings = 10, int segments = 12) {

            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.
            float rings0 = M_PI * -0.5;
            float _z0  = sin(rings0);
            float z0  = r * _z0;
            float r0 =  cos(rings0);
            float rings1 = M_PI * (-0.5 + 1.0f / rings);
            float _z1 = sin(rings1);
            float z1 = r * _z1;
            float r1 = cos(rings1);
            // Push the central point
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);

            GLushort capMiddle = idx++;
            GLushort ringStartIdx = idx;
            GLushort lastRingStartIdx = idx;

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float segment = 2 * M_PI * (float) (j) / segments;
                float x = cos(segment);
                float y = sin(segment);

                float _x1 = x*r1;
                float x1 = _x1*r;
                float _y1 = y*r1;
                float y1 = _y1*r;

                this->vertex_push (so[0]+x1, so[1]+y1, so[2]+z1, this->vertexPositions);
                this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);

                if (!firstseg) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (idx-1);
                    this->indices.push_back (idx++);
                } else {
                    idx++;
                    firstseg = false;
                }
            }
            this->indices.push_back (capMiddle);
            this->indices.push_back (idx-1);
            this->indices.push_back (capMiddle+1);

            // Now add the triangles around the rings
            for (int i = 2; i < rings; i++) {

                rings0 = M_PI * (-0.5 + (float) (i) / rings);
                _z0  = sin(rings0);
                z0  = r * _z0;
                r0 =  cos(rings0);

                for (int j = 0; j < segments; j++) {

                    // "current" segment
                    float segment = 2 * M_PI * (float)j / segments;
                    float x = cos(segment);
                    float y = sin(segment);

                    // One vertex per segment
                    float _x0 = x*r0;
                    float x0 = _x0*r;
                    float _y0 = y*r0;
                    float y0 = _y0*r;

                    // NB: Only add ONE vertex per segment. ALREADY have the first ring!
                    this->vertex_push (so[0]+x0, so[1]+y0, so[2]+z0, this->vertexPositions);
                    // The vertex normal of a vertex that makes up a sphere is
                    // just a normal vector in the direction of the vertex.
                    this->vertex_push (_x0, _y0, _z0, this->vertexNormals);
                    this->vertex_push (sc, this->vertexColors);

                    if (j == segments - 1) {
                        // Last vertex is back to the start
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (idx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (idx++);
                        this->indices.push_back (lastRingStartIdx+segments);
                    } else {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (idx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (idx++);
                        this->indices.push_back (idx);
                    }
                }
                lastRingStartIdx += segments;
            }

            // bottom cap
            rings0 = M_PI * 0.5;
            _z0  = sin(rings0);
            z0  = r * _z0;
            r0 =  cos(rings0);
            // Push the central point of the bottom cap
            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);
            capMiddle = idx++;
            firstseg = true;
            // No more vertices to push, just do the indices for the bottom cap
            ringStartIdx = lastRingStartIdx;
            for (int j = 0; j < segments; j++) {
                if (j != segments - 1) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx++);
                    this->indices.push_back (ringStartIdx);
                } else {
                    // Last segment
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx);
                    this->indices.push_back (lastRingStartIdx);
                }
            }
            // end of sphere calculation
            //cout << "Number of vertexPositions coords: " << (this->vertexPositions.size()/3) << endl;
        }
    };

} // namespace morph

#endif // _VISUALMODEL_H_
