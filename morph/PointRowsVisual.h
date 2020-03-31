#ifndef _POINTSROWVISUAL_H_
#define _POINTSROWVISUAL_H_

#include "GL3/gl3.h"

#include "tools.h"

#include "VisualModel.h"

#include "MathAlgo.h"

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;
#include <array>
using std::array;

namespace morph {

    /*!
     * The template argument Flt is the type of the data which this PointRowsVisual will visualize.
     *
     * A PointsRowVisual is a visualization of a surface which is defined by a set of rows of
     * points, which are aligned perpendicular to one of the 3 Cartesian axes. It was designed to
     * support the Stalefish app, which collects 2-D curves of ISH gene expression and arranges them
     * in a stack.
     */
    template <class Flt>
    class PointRowsVisual : public VisualModel
    {
    public:
        PointRowsVisual(GLuint sp,
                        const vector<array<Flt,3>>* _pointrows,
                        const array<float, 3> _offset,
                        const vector<Flt>* _data,
                        const array<Flt, 2> _scale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->scale = _scale;
            this->pointrows = _pointrows;
            this->data = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Convert datum using our scale into a colour triplet (RGB).
        array<float, 3> datumToColour (Flt datum_in) {
            // Scale the input...
            Flt datum = datum_in * this->scale[0] + this->scale[1];
            datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
            datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
            // ...then turn it into a colour:
            return this->cm.convert (datum);
        }

        //! Do the computations to initialize the vertices that will represent the
        //! surface.
        void initializeVertices (void) {

            //cout << __FUNCTION__ << " called" << endl;

            unsigned int npoints = this->pointrows->size();
            unsigned int ndata = this->data->size();

            if (npoints != ndata) {
                cout << "npoints != ndata, return." << endl;
                return;
            }

            vector<Flt> dcopy = *(this->data);
            if (this->scale[0] == 0.0f && this->scale[1] == 0.0f) {
                // Special 0,0 scale means auto scale data
                dcopy = MathAlgo<Flt>::autoscale (dcopy);
                this->scale[0] = 1.0f;
            }

            // First, need to know which set of points form two, adjacent rows. An assumption we'll
            // accept: The rows are listed in slice-order and the points in each row are listed in
            // position-along-the-curve order.
            unsigned int ib = 0;

            size_t r1 = 0;
            size_t r1_e = 0;
            size_t r2 = 0;
            size_t r2_e = 0;

            size_t prlen = this->pointrows->size();

            // pa is this->pa
            float x = (*pointrows)[r1][pa];
            while (r1_e != prlen && (*pointrows)[r1_e][pa] == x) {
                ++r1_e;
            }
            r2 = r1_e--;
            r2_e = r2;
            x = (*pointrows)[r2][pa];
            while (r2_e != prlen && (*pointrows)[r2_e][pa] == x) {
                ++r2_e;
            }
            r2_e--;
            // Now r1, r1_e, r2 and r2_e all point to the right places
            //cout << "r1: " << r1 << ", r1_e: " << r1_e << endl;
            //cout << "r2: " << r2 << ", r2_e: " << r2_e << endl;
            //cout << "prlen is " << prlen << endl;
            while (r2 != prlen) { // While through all 'rows' - pairs of pointrows
                //cout << "====================================" << endl;
                //cout << "  ROW" << endl;
                //cout << "  r1: " << r1 << ", r1_e: " << r1_e << endl;
                //cout << "  r2: " << r2 << ", r2_e: " << r2_e << endl;
                //cout << "====================================" << endl;

                // Push the first two vertices in the row:
                //cout << "Pushing start vertex (" << (*pointrows)[r1][0] << "," << (*pointrows)[r1][1] << "," << (*pointrows)[r1][2] << ") with value " << dcopy[r1] << endl;
                this->vertex_push ((*pointrows)[r1], this->vertexPositions);
                this->vertex_push (this->datumToColour(dcopy[r1]), this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->indices.push_back (ib);
                ib++;

                //cout << "Pushing start vertex (" << (*pointrows)[r2][0] << "," << (*pointrows)[r2][1] << "," << (*pointrows)[r2][2] << ") with value " << dcopy[r2] << endl;
                this->vertex_push ((*pointrows)[r2], this->vertexPositions);
                this->vertex_push (this->datumToColour(dcopy[r2]), this->vertexColors);
                this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->indices.push_back (ib);
                ib++;

                // Now while through the row pushing the rest of the vertices.
                //cout << "While through the rest, with r2 < r2_e=" << r2_e << endl;
                while (r2 <= r2_e && r1 <= r1_e) {

                    // Now iterate r1 and r2 until we get to the end of the two rows.
                    // vtx is r1, r2. Question is: Is other vertex ++r1 or ++r2?
                    size_t r1n = r1+1;
                    size_t r2n = r2+1;

                    //cout << "**************************" << endl;
                    //cout << "r1: " << r1 << ", r1_e: " << r1_e << endl;
                    //cout << "r2: " << r2 << ", r2_e: " << r2_e << endl;

                    // Increment and make sure we didn't drop off the end
                    if (r1n > r1_e && r2n > r2_e) {
                        r1 = r1n;
                        r2 = r2n;
                        //cout << "Breaking as r1n>r1_e and r2n>r2_e" << endl;
                        break;
                    }

                    //cout << "r1: " << r1 << ", r1n: " << r1n << endl;
                    //cout << "r2: " << r2 << ", r2n: " << r2n << endl;

                    bool completed_end_tri = false;
                    bool must_be_r1n = false;
                    bool must_be_r2n = false;
                    if (r1n > r1_e) {
                        // Can't add this one, only r2n is possible
                        must_be_r2n = true;
                        completed_end_tri = true;
                    }
                    if (r2n > r2_e) {
                        // Can't add this one, only r1n is possible and must be at end of row
                        must_be_r1n = true;
                        completed_end_tri = true;
                    }

                    if (must_be_r1n) {
                        must_be_r2n = false;
                    } else if (must_be_r2n) {
                        must_be_r1n = false;
                    } else {
                        // Compute distances to compute angles to decide
                        float r1_to_r2_sq = MathAlgo<float>::distance_sq ((*pointrows)[r1], (*pointrows)[r2]);

                        float r1_to_r1n_sq = MathAlgo<float>::distance_sq ((*pointrows)[r1], (*pointrows)[r1n]);
                        float r1_to_r2n_sq = MathAlgo<float>::distance_sq ((*pointrows)[r1], (*pointrows)[r2n]);
                        float r2_to_r1n_sq = MathAlgo<float>::distance_sq ((*pointrows)[r2], (*pointrows)[r1n]);
                        float r2_to_r2n_sq = MathAlgo<float>::distance_sq ((*pointrows)[r2], (*pointrows)[r2n]);

                        float r1_to_r1n = sqrt(r1_to_r1n_sq);
                        float r1_to_r2n = sqrt(r1_to_r2n_sq);
                        float r2_to_r1n = sqrt(r2_to_r1n_sq);
                        float r2_to_r2n = sqrt(r2_to_r2n_sq);

                        float asq = r1_to_r2_sq;
                        float bsq = r2_to_r1n_sq;
                        float b = r2_to_r1n;
                        float csq = r1_to_r1n_sq;
                        float c = r1_to_r1n;
                        float alpha1 = acos ((bsq + csq - asq)/(2*b*c));

                        bsq = r2_to_r2n_sq;
                        b = r2_to_r2n;
                        csq = r1_to_r2n_sq;
                        c = r1_to_r2n;
                        float alpha2 = acos ((bsq + csq - asq)/(2*b*c));

                        if (alpha2 < alpha1) {
                            // r1
                            must_be_r1n = true;
                            must_be_r2n = false;
                        } else {
                            must_be_r1n = false;
                            must_be_r2n = true;
                        }
                    }

                    if (must_be_r1n) {
                        // r1 is the next
                        r1 = r1n;
                        //cout << "Pushing r1 vertex (" << (*pointrows)[r1][0] << "," << (*pointrows)[r1][1] << "," << (*pointrows)[r1][2] << ") with value " << dcopy[r1] << endl;
                        this->vertex_push ((*pointrows)[r1], this->vertexPositions);
                        this->vertex_push (this->datumToColour(dcopy[r1]), this->vertexColors);
                        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                        this->indices.push_back (ib);
                        ib++;

                    } else {
                        // r2 is next
                        r2 = r2n;
                        //cout << "Pushing r2 vertex (" << (*pointrows)[r2][0] << "," << (*pointrows)[r2][1] << "," << (*pointrows)[r2][2] << ") with value " << dcopy[r2] << endl;
                        this->vertex_push ((*pointrows)[r2], this->vertexPositions);
                        this->vertex_push (this->datumToColour(dcopy[r2]), this->vertexColors);
                        this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                        this->indices.push_back (ib);
                        ib++;
                    }

                    if (completed_end_tri == true) {
                        //cout << "Completed the end triangle";
                        break;
                    }

                    // Next tri:
                    //cout << "Next tri." << endl;
                    //cout << "Pushing vertex (" << (*pointrows)[r1][0] << "," << (*pointrows)[r1][1] << "," << (*pointrows)[r1][2] << ") with value " << dcopy[r1] << endl;
                    this->vertex_push ((*pointrows)[r1], this->vertexPositions);
                    this->vertex_push (this->datumToColour(dcopy[r1]), this->vertexColors);
                    this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                    this->indices.push_back (ib);
                    ib++;

                    //cout << "Pushing vertex (" << (*pointrows)[r2][0] << "," << (*pointrows)[r2][1] << "," << (*pointrows)[r2][2] << ") with value " << dcopy[r2] << endl;
                    this->vertex_push ((*pointrows)[r2], this->vertexPositions);
                    this->vertex_push (this->datumToColour(dcopy[r2]), this->vertexColors);
                    this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                    this->indices.push_back (ib);
                    ib++;
                }

                // On to the next rows:
                r1 = r1_e + 1;
                r1_e = r1;
                r2 = r2_e; ++r2;
                r2_e = r2;

                //cout << "Next r1 is: " << r1 << endl;
                //cout << "Next r2 is: " << r2 << endl;
                //cout << "ib is now " << ib << endl;
                if (r2 == prlen) {
                    //cout << "No more rows, break." << endl;
                    break;
                }

                x = (*pointrows)[r1][pa];
                while (r1_e != prlen && (*pointrows)[r1_e][pa] == x) {
                    ++r1_e;
                }
                r1_e--;
                r2_e = r2;
                x = (*pointrows)[r2][pa];
                while (r2_e != prlen && (*pointrows)[r2_e][pa] == x) {
                    ++r2_e;
                }
                r2_e--;
                // Now r1, r1_e, r2 and r2_e all point to the right places
                //cout << "Next r1,r1, r1_e, r2_e:" << endl;
                //cout << "r1: " << r1 << ", r1_e: " << r1_e << endl;
                //cout << "r2: " << r2 << ", r2_e: " << r2_e << endl;
            }
        }

        //! Update the data and re-compute the vertices.
        void updateData (const vector<Flt>* _data, const array<Flt, 2> _scale) {
            this->scale = _scale;
            this->data = _data;
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->initializeVertices();
            // Now re-set up the VBOs
            int sz = this->indices.size() * sizeof(VBOint);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[posnVBO], this->vertexPositions, posnLoc);
            this->setupVBO (this->vbos[normVBO], this->vertexNormals, normLoc);
            this->setupVBO (this->vbos[colVBO], this->vertexColors, colLoc);
        }

        /*!
         * The relevant colour map. Change the type/hue of this colour map object to
         * generate different types of map.
         */
        ColourMap<Flt> cm;

        //! The linear scaling for the colour is y1 = m1 x + c1 (m1 = scale[0] and c1 =
        //! scale[1]) If all entries of scale are static_cast<Flt>(0), then auto-scale.
        array<Flt, 2> scale;

    private:
        //! The PointRows to visualize. This is a vector of 3D coordinates that define the vertices
        //! of the triangular mesh.
        const vector<array<Flt,3>>* pointrows;

        //! The data to visualize as colour (modulated by the linear scaling
        //! provided in this->scale)
        const vector<Flt>* data;

        //! Which axis are we perpendicular to?
        size_t pa = 0;
    };

} // namespace morph

#endif // _POINTROWSVISUAL_H_
