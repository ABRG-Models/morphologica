#ifndef _COORDARROWS_H_
#define _COORDARROWS_H_

#include "VisualModel.h"

#include "MathConst.h"

#include <iostream>
using std::cout;
using std::endl;

#include <array>
using std::array;

#if 0
// Add +, - operators to std::array
template<class T, size_t N>
class Array : public array<T, N>
{
public:
    using array<T, N>::array;
    Array operator+(Array const& rhs) const {
        Array res;
        transform (this->begin(), this->end(), rhs.begin(), res.begin(), std::plus);
        return res;
    }
    Array operator-(Array const& rhs) const {
        Array res;
        transform (this->begin(), this->end(), rhs.begin(), res.begin(), std::minus);
        return res;
    }
};
#endif

namespace morph {

    //! This class creates the vertices for a set of coordinate arrows to be rendered
    //! in a 3-D scene.
    class CoordArrows : public VisualModel
    {
    public:
        CoordArrows (void) {
            this->scale = {1.0, 1.0, 1.0};
            this->offset = {0.0, 0.0, 0.0};
        }

        CoordArrows(GLuint sp,
                    const array<float, 3> _offset,
                    const array<float, 3> _scale) {
            this->init (sp, _offset, _scale);
        }

        ~CoordArrows () {}

        void init (GLuint sp,
                   const array<float, 3> _offset,
                   const array<float, 3> _scale) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);

            this->scale = _scale;

            // Do the computations to initialize the vertices that well
            // represent the HexGrid.
            this->initializeVertices();

            this->postVertexInit();
        }

        //! Initialize vertex buffer objects and vertex array object.
        //@{
        //! Initialize as triangled. Gives a smooth surface with much
        //! less comput than initializeVerticesHexesInterpolated.
        void initializeVertices (void) {

            // The indices index
            GLushort idx = 0;

            // Draw four spheres to make up the coord frame
            array<float, 3> reloffset = this->offset;
            this->computeSphere (idx, this->offset, {1.0,1.0,1.0}, this->scale[0]/20.0);

            // x
            reloffset[0] += this->scale[0];
            this->computeSphere (idx, reloffset, {1.0,0.0,0.0}, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, {1.0,0.0,0.0}, this->scale[0]/80.0);
#if 0
            // y
            reloffset[0] -= this->scale[0];
            reloffset[1] += this->scale[1];
            this->computeSphere (idx, reloffset, {0.0,1.0,0.0}, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, {0.0,1.0,0.0}, this->scale[0]/80.0);

            // z
            reloffset[1] -= this->scale[1];
            reloffset[2] += this->scale[2];
            this->computeSphere (idx, reloffset, {0.0,0.0,1.0}, this->scale[0]/40.0);
            this->computeTube (idx, this->offset, reloffset, {0.0,0.0,1.0}, this->scale[0]/80.0);
#endif
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
        void computeTube (GLushort& idx, array<float, 3> start, array<float, 3> end, array<float, 3> col,
                          float r = 1.0f, int segments = 12) {

            // First cap, draw as a triangle fan, but record indices so that
            // we only need a single call to glDrawElements.

            // The vector from start to end defines a vector and a plane. Find a 'circle' of points in that plane.
            Vector3<float> vstart (start);
            Vector3<float> vend (end);
            cout << "Compute tube from " << vstart.asString() << "to " << vend.asString() << endl;
            Vector3<float> v = vend - vstart;
            v.renormalize();
            cout << "Normal vector v is " << v.asString() << endl;

            // circle in a plane defined by a point (v0 = vstart or vend) and a normal
            // (v) can be found: Choose random vector vr. A vector inplane = vr ^
            // v. The unit in-plane vector is inplane.normalise. Can now use that
            // vector in the plan to define a point on the circle.
            Vector3<float> rand_vec;
            rand_vec.randomize();
            Vector3<float> inplane = rand_vec * v;
            inplane.renormalize();
            cout << "in-plane vector is " << inplane.asString() << endl;

            // Now use parameterization of circle inplane = p1-x1 and
            // c1(t) = ( (p1-x1).normalized sin(t) + v.normalized cross (p1-x1).normalized * cos(t) )
            // c1(t) = ( inplane sin(t) + v * inplane * cos(t)
            Vector3<float> v_x_inplane = v * inplane;
            cout << "v ^ inplane vector is " << v_x_inplane.asString() << endl;
            // Point on circle: Vector3<float> c = inplane * sin(t) + v_x_inplane * cos(t);

            // Push the central point of the start cap - this is at location vstart
            this->vertex_push (vstart, this->vertexPositions);
            cout << "Central point of vstart cap is " << vstart.asString() << endl;
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector3<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                cout << "point on vstart cap is " << c.asString() << endl;
                this->vertex_push (-v, this->vertexNormals); // -v
                this->vertex_push (col, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = j * morph::TWO_PI_F/(float)segments;
                Vector3<float> c = inplane * sin(t) * r + v_x_inplane * cos(t) * r;
                this->vertex_push (vend+c, this->vertexPositions);
                cout << "point on vend cap is " << c.asString() << endl;
                this->vertex_push (v, this->vertexNormals); // +v
                this->vertex_push (col, this->vertexColors);
            }

            // Bottom cap. Push centre vertex as the last vertex.
            this->vertex_push (vend, this->vertexPositions);
            cout << "vend cap is " << vend.asString() << endl;
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (col, this->vertexColors);

            // Note: number of vertices = segments * 2 + 2.
            int nverts = segments * 2 + 2;

            // After creating vertices, push all the indices.
            GLushort capMiddle = idx;
            GLushort capStartIdx = idx + 1;
            GLushort endMiddle = idx + (GLushort)nverts;
            GLushort endStartIdx = endMiddle + 1;

            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            for (int j = 0; j < segments; j++) {
                // Two triangles per side; 1:
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
                this->indices.push_back (endStartIdx + j);
                // 2:
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }

            // bottom cap
            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            // Last one
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);


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
            cout << "Number of vertexPositions coords: " << (this->vertexPositions.size()/3) << endl;
        }

        //! The lengths of the x, y and z arrows.
        array<float, 3> scale;
    };

} // namespace morph

#endif // _COORDARROWS_H_
