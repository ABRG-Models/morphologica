#pragma once

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/MathAlgo.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <iostream>
#include <vector>
#include <array>

namespace morph {

    /*!
     * The template argument Flt is the type of the data which this PointRowsVisual will visualize.
     *
     * A PointRowsVisual is a visualization of a surface which is defined by a set of rows of
     * points, which are aligned perpendicular to one of the 3 Cartesian axes. It was designed to
     * support the Stalefish app, which collects 2-D curves of ISH gene expression and arranges them
     * in a stack.
     */
    template <typename Flt, int glver = morph::gl::version_4_1>
    class PointRowsVisual : public VisualDataModel<Flt, glver>
    {
    public:
        PointRowsVisual(std::vector<vec<float,3>>* _pointrows,
                        const vec<float, 3> _offset,
                        const std::vector<Flt>* _data,
                        const Scale<Flt>& cscale,
                        ColourMapType _cmt,
                        const float _hue = 0.0f)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->colourScale = cscale;

            this->dataCoords = _pointrows;
            this->scalarData = _data;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);
        }

        //! Convert datum using our scale into a colour triplet (RGB).
        std::array<float, 3> datumToColour (Flt datum_in)
        {
            // Scale the input...
            Flt datum = datum_in * this->scale[0] + this->scale[1];
            datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
            datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
            // ...then turn it into a colour:
            return this->cm.convert (datum);
        }

        //! Do the computations to initialize the vertices that will represent the
        //! surface.
        void initializeVertices()
        {
            unsigned int npoints = this->dataCoords->size();
            unsigned int ndata = this->scalarData->size();

            if (npoints != ndata) {
                std::cout << "npoints != ndata, return." << std::endl;
                return;
            }

            std::vector<Flt> dcopy = *(this->scalarData);
            this->colourScale.do_autoscale = true;
            this->colourScale.transform (*this->scalarData, dcopy);

            // First, need to know which set of points form two, adjacent rows. An assumption we'll
            // accept: The rows are listed in slice-order and the points in each row are listed in
            // position-along-the-curve order.

            size_t r1 = 0;
            size_t r1_e = 0;
            size_t r2 = 0;
            size_t r2_e = 0;

            size_t prlen = this->dataCoords->size();

            // pa is this->pa
            float x = (*this->dataCoords)[r1][pa];
            while (r1_e != prlen && (*this->dataCoords)[r1_e][pa] == x) {
                ++r1_e;
            }
            r2 = r1_e--;
            r2_e = r2;
            x = (*this->dataCoords)[r2][pa];
            while (r2_e != prlen && (*this->dataCoords)[r2_e][pa] == x) {
                ++r2_e;
            }
            r2_e--;
            // Now r1, r1_e, r2 and r2_e all point to the right places
            //std::cout << "r1: " << r1 << ", r1_e: " << r1_e << std::endl;
            //std::cout << "r2: " << r2 << ", r2_e: " << r2_e << std::endl;
            //std::cout << "prlen is " << prlen << std::endl;
            morph::vec<float> v0, v1, v2;
            while (r2 != prlen) { // While through all 'rows' - pairs of pointrows
                //std::cout << "====================================" << std::endl;
                //std::cout << "  ROW" << std::endl;
                //std::cout << "  r1: " << r1 << ", r1_e: " << r1_e << std::endl;
                //std::cout << "  r2: " << r2 << ", r2_e: " << r2_e << std::endl;
                //std::cout << "====================================" << std::endl;

                // Push the first two vertices in the row:
                // Start vertex 1
                v1 = (*this->dataCoords)[r1];
                this->vertex_push ((*this->dataCoords)[r1], this->vertexPositions);
                this->vertex_push (this->cm.convert(dcopy[r1]), this->vertexColors);
                //this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->indices.push_back (this->idx);
                this->idx++;
                // Start vertex 2
                v2 = (*this->dataCoords)[r2];
                this->vertex_push ((*this->dataCoords)[r2], this->vertexPositions);
                this->vertex_push (this->cm.convert(dcopy[r2]), this->vertexColors);
                //this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
                this->indices.push_back (this->idx);
                this->idx++;

                // Let v0 be:
                morph::vec<float> vnorm = {0.0f, 0.0f, 1.0f};
                if (r1+1 < r1_e) {
                    v0 = (*this->dataCoords)[r1+1];
                    // Compute normal
                    morph::vec<float> plane1 = v1 - v0;
                    morph::vec<float> plane2 = v2 - v0;
                    vnorm = plane2.cross (plane1);
                    vnorm.renormalize();
                }
                // Push normal twice for the two start vertices:
                this->vertex_push (vnorm, this->vertexNormals);
                this->vertex_push (vnorm, this->vertexNormals);

                // Now while through the row pushing the rest of the vertices.
                //std::cout << "While through the rest, with r2 < r2_e=" << r2_e << std::endl;
                while (r2 <= r2_e && r1 <= r1_e) {

                    // Now iterate r1 and r2 until we get to the end of the two rows.
                    // vtx is r1, r2. Question is: Is other vertex ++r1 or ++r2?
                    size_t r1n = r1+1;
                    size_t r2n = r2+1;

                    //std::cout << "**************************" << std::endl;
                    //std::cout << "r1: " << r1 << ", r1_e: " << r1_e << std::endl;
                    //std::cout << "r2: " << r2 << ", r2_e: " << r2_e << std::endl;

                    // Increment and make sure we didn't drop off the end
                    if (r1n > r1_e && r2n > r2_e) {
                        r1 = r1n;
                        r2 = r2n;
                        //std::cout << "Breaking as r1n>r1_e and r2n>r2_e" << std::endl;
                        break;
                    }

                    //std::cout << "r1: " << r1 << ", r1n: " << r1n << std::endl;
                    //std::cout << "r2: " << r2 << ", r2n: " << r2n << std::endl;

                    bool completed_end_tri = false;
                    bool must_be_r1n = false;
                    bool must_be_r2n = false;
                    if (r1n > r1_e) {
                        // Can't add this one, only r2n is possthis->idxle
                        must_be_r2n = true;
                        completed_end_tri = true;
                    }
                    if (r2n > r2_e) {
                        // Can't add this one, only r1n is possthis->idxle and must be at end of row
                        must_be_r1n = true;
                        completed_end_tri = true;
                    }

                    if (must_be_r1n) {
                        must_be_r2n = false;
                    } else if (must_be_r2n) {
                        must_be_r1n = false;
                    } else {
                        // Compute distances to compute angles to decide
                        float r1_to_r2_sq = MathAlgo::distance_sq<float, 3> ((*this->dataCoords)[r1], (*this->dataCoords)[r2]);

                        float r1_to_r1n_sq = MathAlgo::distance_sq<float, 3> ((*this->dataCoords)[r1], (*this->dataCoords)[r1n]);
                        float r1_to_r2n_sq = MathAlgo::distance_sq<float, 3> ((*this->dataCoords)[r1], (*this->dataCoords)[r2n]);
                        float r2_to_r1n_sq = MathAlgo::distance_sq<float, 3> ((*this->dataCoords)[r2], (*this->dataCoords)[r1n]);
                        float r2_to_r2n_sq = MathAlgo::distance_sq<float, 3> ((*this->dataCoords)[r2], (*this->dataCoords)[r2n]);

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
                        // r1 vertex is the next
                        r1 = r1n;
                        v0 = (*this->dataCoords)[r1];
                        this->vertex_push ((*this->dataCoords)[r1], this->vertexPositions);
                        this->vertex_push (this->cm.convert(dcopy[r1]), this->vertexColors);
                        this->indices.push_back (this->idx);
                        this->idx++;

                    } else {
                        // r2 vertex is next
                        r2 = r2n;
                        v0 = (*this->dataCoords)[r2];
                        this->vertex_push ((*this->dataCoords)[r2], this->vertexPositions);
                        this->vertex_push (this->cm.convert(dcopy[r2]), this->vertexColors);
                        this->indices.push_back (this->idx);
                        this->idx++;
                    }

                    // Compute normal and push one
                    morph::vec<float> plane1 = v1 - v0;
                    morph::vec<float> plane2 = v2 - v0;
                    morph::vec<float> vnorm = plane2.cross (plane1);
                    vnorm.renormalize();
                    this->vertex_push (vnorm, this->vertexNormals);

                    if (completed_end_tri == true) {
                        //std::cout << "Completed the end triangle";
                        break;
                    }

                    // Next tri:
                    v1 = (*this->dataCoords)[r1];
                    this->vertex_push ((*this->dataCoords)[r1], this->vertexPositions);
                    this->vertex_push (this->cm.convert(dcopy[r1]), this->vertexColors);
                    this->vertex_push (vnorm, this->vertexNormals);
                    this->indices.push_back (this->idx);
                    this->idx++;

                    v2 = (*this->dataCoords)[r2];
                    this->vertex_push ((*this->dataCoords)[r2], this->vertexPositions);
                    this->vertex_push (this->cm.convert(dcopy[r2]), this->vertexColors);
                    this->vertex_push (vnorm, this->vertexNormals);
                    this->indices.push_back (this->idx);
                    this->idx++;
                }

                // On to the next rows:
                r1 = r1_e + 1;
                r1_e = r1;
                r2 = r2_e; ++r2;
                r2_e = r2;

                //std::cout << "Next r1 is: " << r1 << std::endl;
                //std::cout << "Next r2 is: " << r2 << std::endl;
                //std::cout << "this->idx is now " << this->idx << std::endl;
                if (r2 == prlen) {
                    //std::cout << "No more rows, break." << std::endl;
                    break;
                }

                x = (*this->dataCoords)[r1][pa];
                while (r1_e != prlen && (*this->dataCoords)[r1_e][pa] == x) {
                    ++r1_e;
                }
                r1_e--;
                r2_e = r2;
                x = (*this->dataCoords)[r2][pa];
                while (r2_e != prlen && (*this->dataCoords)[r2_e][pa] == x) {
                    ++r2_e;
                }
                r2_e--;
                // Now r1, r1_e, r2 and r2_e all point to the right places
                //std::cout << "Next r1,r1, r1_e, r2_e:" << std::endl;
                //std::cout << "r1: " << r1 << ", r1_e: " << r1_e << std::endl;
                //std::cout << "r2: " << r2 << ", r2_e: " << r2_e << std::endl;
            }
        }

    private:
        //! Which axis are we perpendicular to?
        size_t pa = 0;
    };

} // namespace morph
