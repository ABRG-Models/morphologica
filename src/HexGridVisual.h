#ifndef _HEXGRIDVISUAL_H_
#define _HEXGRIDVISUAL_H_

#include "GL3/gl3.h"
#include "GL/glext.h"

#include "tools.h"

#include "HexGrid.h"

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

/*!
 * Macros for testing neighbours. The step along for neighbours on the
 * rows above/below is given by:
 *
 * Dest  | step
 * ----------------------
 * NNE   | +rowlen
 * NNW   | +rowlen - 1
 * NSW   | -rowlen
 * NSE   | -rowlen + 1
 */
//@{
#define NE(hi) (this->hg->d_ne[hi])
#define HAS_NE(hi) (this->hg->d_ne[hi] == -1 ? false : true)

#define NW(hi) (this->hg->d_nw[hi])
#define HAS_NW(hi) (this->hg->d_nw[hi] == -1 ? false : true)

#define NNE(hi) (this->hg->d_nne[hi])
#define HAS_NNE(hi) (this->hg->d_nne[hi] == -1 ? false : true)

#define NNW(hi) (this->hg->d_nnw[hi])
#define HAS_NNW(hi) (this->hg->d_nnw[hi] == -1 ? false : true)

#define NSE(hi) (this->hg->d_nse[hi])
#define HAS_NSE(hi) (this->hg->d_nse[hi] == -1 ? false : true)

#define NSW(hi) (this->hg->d_nsw[hi])
#define HAS_NSW(hi) (this->hg->d_nsw[hi] == -1 ? false : true)
//@}

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

namespace morph {

    //! Forward declaration of a templated Visual class
    class Visual;

    //! The locations for the position, normal and colour vertex attributes in the GLSL program
    enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2 };

    //! The template arguemnt Flt is the type of the data which this HexGridVisual will visualize.
    template <class Flt>
    class HexGridVisual
    {
    public:
        HexGridVisual(GLuint sp,
                      const HexGrid* _hg,
                      const array<float, 3> _offset,
                      const vector<Flt>* _data,
                      const array<Flt, 4> _scale) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->scale = _scale;
            this->hg = _hg;
            this->data = _data;

            // Do the computations to initialize the vertices that well
            // represent the HexGrid.
            this->initializeVerticesHexesInterpolated();
            //this->initializeVerticesTris();

            // Create vertex array object
            glCreateVertexArrays (1, &this->vao);
            glBindVertexArray (this->vao);

            // Allocate/create the vertex buffer objects
            this->vbos = new GLuint[numVBO];
            glCreateBuffers (numVBO, this->vbos);

            // Set up the indices buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbos[idxVBO]);
            int sz = this->indices.size() * sizeof(VBOint);
            cout << "indices sz = " << sz << " bytes" << endl;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);

            // Possibly release (unbind) the vertex buffers (but not index buffer)
            // Possible glVertexAttribPointer and glEnableVertexAttribArray?
            glUseProgram (shaderprog);

            // Here's how to unbind the VAO
            //glBindVertexArray(0);
        }

        ~HexGridVisual() {
            // destroy buffers
            glDeleteBuffers (4, vbos);
            delete (this->vbos);
        }

        //! Initialize vertex buffer objects and vertex array object.
        //@{
        //! Initialize as triangled. Gives a smooth surface with much
        //! less comput than initializeVerticesHexesInterpolated.
        void initializeVerticesTris (void) {
            unsigned int nhex = this->hg->num();
            for (unsigned int hi = 0; hi < nhex; ++hi) {
                this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi], (*this->data)[hi], this->vertexPositions);
                this->vertex_push (morph::Tools::getJetColorF((double)(*this->data)[hi]+0.5), this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            }

            // Build indices based on neighbour relations in the HexGrid
            for (unsigned int hi = 0; hi < nhex; ++hi) {
                if (HAS_NNE(hi) && HAS_NE(hi)) {
                    cout << "1st triangle " << hi << "->" << NNE(hi) << "->" << NE(hi) << endl;
                    this->indices.push_back (hi);
                    this->indices.push_back (NNE(hi));
                    this->indices.push_back (NE(hi));
                }

                if (HAS_NW(hi) && HAS_NSW(hi)) {
                    cout << "2nd triangle " << hi << "->" << NW(hi) << "->" << NSW(hi) << endl;
                    this->indices.push_back (hi);
                    this->indices.push_back (NW(hi));
                    this->indices.push_back (NSW(hi));
                }
            }
        }

        //! Apply scaling for Z position
        inline Flt sc (const Flt& datum) {
            return (datum * this->scale[0] + this->scale[1]);
        }

        //! Update the data and re-compute the vertices.
        void updateData (const vector<Flt>* _data, const array<Flt, 4> _scale) {
            this->scale = _scale;
            this->data = _data;
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->initializeVerticesHexesInterpolated();
            // Now re-set up the VBOs
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);
        }

        //! Initialize as hexes, with z position of each of the 6
        //! outer edges of the hexes interpolated, but a single colour
        //! for each hex. Gives a smooth surface.
        void initializeVerticesHexesInterpolated (void) {
            float sr = this->hg->getSR();
            float vne = this->hg->getVtoNE();
            float lr = this->hg->getLR();

            unsigned int nhex = this->hg->num();
            unsigned int idx = 0;

            Flt datumC = static_cast<Flt>(0.0);   // datum at the centre
            Flt datumNE = static_cast<Flt>(0.0);  // datum at the hex to the east.
            Flt datumNNE = static_cast<Flt>(0.0); // etc
            Flt datumNNW = static_cast<Flt>(0.0);
            Flt datumNW = static_cast<Flt>(0.0);
            Flt datumNSW = static_cast<Flt>(0.0);
            Flt datumNSE = static_cast<Flt>(0.0);

            Flt datum = static_cast<Flt>(0.0);
            Flt third = static_cast<Flt>(0.33333333333333);
            Flt half = static_cast<Flt>(0.5);
            for (unsigned int hi = 0; hi < nhex; ++hi) {

                // Compute the linear scalings. Could do this once only and have a this->scaledData member
                datumC = this->sc((*this->data)[hi]);
                datumNE = this->sc((*this->data)[NE(hi)]);   // datum Neighbour East
                datumNNE = this->sc((*this->data)[NNE(hi)]); // datum Neighbour North East
                datumNNW = this->sc((*this->data)[NNW(hi)]); // etc
                datumNW = this->sc((*this->data)[NW(hi)]);
                datumNSW = this->sc((*this->data)[NSW(hi)]);
                datumNSE = this->sc((*this->data)[NSE(hi)]);

                // Use a single colour for each hex, even though hex z positions are
                // interpolated. Do the _colour_ scaling:
                datum = (*this->data)[hi] * this->scale[2] + this->scale[3];
                datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
                datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
                // And turn it into a colour:
                array<float, 3> clr = morph::Tools::getJetColorF((double)datum);

                // First push the 7 positions of the triangle vertices, starting with the centre
                this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi], datumC, this->vertexPositions);

                // NE vertex
                if (HAS_NNE(hi) && HAS_NE(hi)) {
                    // Compute mean of this->data[hi] and NE and E hexes
                    datum = third * (datumC + datumNNE + datumNE);
                } else if (HAS_NNE(hi) || HAS_NE(hi)) {
                    if (HAS_NNE(hi)) {
                        datum = half * (datumC + datumNNE);
                    } else {
                        datum = half * (datumC + datumNE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);

                // SE vertex
                if (HAS_NE(hi) && HAS_NSE(hi)) {
                    datum = third * (datumC + datumNE + datumNSE);
                } else if (HAS_NE(hi) || HAS_NSE(hi)) {
                    if (HAS_NE(hi)) {
                        datum = half * (datumC + datumNE);
                    } else {
                        datum = half * (datumC + datumNSE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi]+sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);

                // S
                if (HAS_NSE(hi) && HAS_NSW(hi)) {
                    datum = third * (datumC + datumNSE + datumNSW);
                } else if (HAS_NSE(hi) || HAS_NSW(hi)) {
                    if (HAS_NSE(hi)) {
                        datum = half * (datumC + datumNSE);
                    } else {
                        datum = half * (datumC + datumNSW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]-lr, datum, this->vertexPositions);

                // SW
                if (HAS_NW(hi) && HAS_NSW(hi)) {
                    datum = third * (datumC + datumNW + datumNSW);
                } else if (HAS_NW(hi) || HAS_NSW(hi)) {
                    if (HAS_NW(hi)) {
                        datum = half * (datumC + datumNW);
                    } else {
                        datum = half * (datumC + datumNSW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]-vne, datum, this->vertexPositions);

                // NW
                if (HAS_NNW(hi) && HAS_NW(hi)) {
                    datum = third * (datumC + datumNNW + datumNW);
                } else if (HAS_NNW(hi) || HAS_NW(hi)) {
                    if (HAS_NNW(hi)) {
                        datum = half * (datumC + datumNNW);
                    } else {
                        datum = half * (datumC + datumNW);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi]-sr, this->hg->d_y[hi]+vne, datum, this->vertexPositions);

                // N
                if (HAS_NNW(hi) && HAS_NNE(hi)) {
                    datum = third * (datumC + datumNNW + datumNNE);
                } else if (HAS_NNW(hi) || HAS_NNE(hi)) {
                    if (HAS_NNW(hi)) {
                        datum = half * (datumC + datumNNW);
                    } else {
                        datum = half * (datumC + datumNNE);
                    }
                } else {
                    datum = datumC;
                }
                this->vertex_push (this->hg->d_x[hi], this->hg->d_y[hi]+lr, datum, this->vertexPositions);

                // All normal point up
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);

                // Seven vertices with the same colour
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);
                this->vertex_push (clr, this->vertexColors);

                // Define indices now to produce the 6 triangles in the hex
                this->indices.push_back (idx+1);
                this->indices.push_back (idx);
                this->indices.push_back (idx+2);

                this->indices.push_back (idx+2);
                this->indices.push_back (idx);
                this->indices.push_back (idx+3);

                this->indices.push_back (idx+3);
                this->indices.push_back (idx);
                this->indices.push_back (idx+4);

                this->indices.push_back (idx+4);
                this->indices.push_back (idx);
                this->indices.push_back (idx+5);

                this->indices.push_back (idx+5);
                this->indices.push_back (idx);
                this->indices.push_back (idx+6);

                this->indices.push_back (idx+6);
                this->indices.push_back (idx);
                this->indices.push_back (idx+1);

                idx += 7; // 7 vertices (each of 3 floats for x/y/z), 18 indices.
            }
        }

        //! Initialize as hexes, with a step quad between each
        //! hex. Might look cool. Writeme.
        void initializeVerticesHexesStepped (void) {}
        //@}

        //! Render the HexGridVisual
        void render (void) {
            glBindVertexArray (this->vao);
            //cout << "Render " << this->indices.size() << " vertices" << endl;
            glDrawElements (GL_TRIANGLES, this->indices.size(), VBO_ENUM_TYPE, 0);
            glBindVertexArray(0);
        }

        //! The model-specific view matrix.
        TransformMatrix<float> viewmatrix;

        //! Linear scaling which should be applied to the (scalar value of the)
        //! data. y = mx + c, with scale[0] == m and scale[1] == c. The linear scaling
        //! for the colour is y1 = m1 x + c1 (m1 = scale[2] and c1 = scale[3])
        array<Flt, 4> scale;

        //! Setter for offset, also updates viewmatrix.
        void setOffset (const array<float, 3>& _offset) {
            this->offset = _offset;
            this->viewmatrix.setIdentity();
            this->viewmatrix.translate (this->offset);
        }

        //! Shift the offset, also updates viewmatrix.
        void shiftOffset (const array<float, 3>& _offset) {
            this->offset += _offset; // Why do I need the offset member? I don't
            this->viewmatrix.translate (this->offset);
        }

    private:

        //! The offset of this HexGridVisual. Note that this is not incorporated into
        //! the computation of the vertices, but is instead applied when the object is
        //! rendered as part of the model->world transformation.
        array<float, 3> offset;

        //! This enum contains the positions within the vbo array of the different vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        //! The parent Visual object - provides access to the shader prog
        const Visual* parent;

        //! The HexGrid to visualize
        const HexGrid* hg;

        //! The data to visualize as z/colour (modulated by the linear scaling
        //! provided in this->scale)
        const vector<Flt>* data;

        //! A copy of the reference to the shader program
        GLuint shaderprog;

        // Add a way to control the scaling scheme here.

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

#endif // _HEXGRIDVISUAL_H_
