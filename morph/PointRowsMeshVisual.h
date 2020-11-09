#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/MathAlgo.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/ColourMap.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

namespace morph {

    /*!
     * The template argument Flt is the type of the data which this PointRowsMeshVisual
     * will visualize.
     *
     * A PointRowsMeshVisual is a visualization of a surface which is defined by a set
     * of rows of points, which are aligned perpendicular to one of the 3 Cartesian
     * axes. It was designed to support the Stalefish app, which collects 2-D curves of
     * ISH gene expression and arranges them in a stack.
     *
     * This differs from PointRowsVisual in that it draw the mesh of with spheres at the
     * vertices and lines between the vertices. It therefore creates a lot more vertices
     * than PointRowsVisual.
     *
     * \param sp The shader program
     *
     * \param _pointrows The vector of coordinates of the points to visualise
     *
     * \param _offset The offset within the morph::Visual scene at which the model will
     * be drawn (used when rendering, not when creating the model's vertices)
     *
     * \param _data The data, which can be used to colour the spheres or rods
     *
     * \param cscale colour scale for scaling data into colour
     *
     * \param _cmt The mesh's colour map type
     *
     * \param _hue The mesh's colour hue, used if _cmt is ColourMapType::Fixed or ::Monochrome
     *
     * \param _sat The mesh's colour saturation, used if _cmt is ColourMapType::Fixed
     *
     * \param _val The mesh's colour value, used if _cmt is ColourMapType::Fixed
     *
     * \param _radius The radius of the rods making up the mesh.
     *
     * \param _cmt_sph The sphere's colour map type
     *
     * \param _hue_sph The sphere's colour hue, used if _cmt_sph is ColourMapType::Fixed or ::Monochrome
     *
     * \param _sat_sph The sphere's colour saturation, used if _cmt_sph is ColourMapType::Fixed
     *
     * \param _val_sph The sphere's colour value, used if _cmt_sph is ColourMapType::Fixed
     *
     * \param _radius_sph The radius of the spheres making up the points. 3 times
     * _radius is good to see the spheres. Make this 0 to omit the spheres.
     *
     */
    template <typename Flt>
    class PointRowsMeshVisual : public VisualDataModel<Flt>
    {
    public:
        PointRowsMeshVisual(GLuint sp,
                            std::vector<Vector<float,3>>* _pointrows,
                            const Vector<float, 3> _offset,
                            const std::vector<Flt>* _data,
                            const Scale<Flt>& cscale,
                            ColourMapType _cmt, // mesh
                            const float _hue,
                            const float _sat,
                            const float _val,
                            const float _radius,
                            ColourMapType _cmt_sph, // spheres
                            const float _hue_sph,
                            const float _sat_sph,
                            const float _val_sph,
                            const float _radius_sph) {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->radius = _radius;
            this->sradius = _radius_sph;

            this->colourScale = cscale;

            this->dataCoords = _pointrows;
            this->scalarData = _data;

            // Perhaps I should have just passed in 2 colour maps!
            this->cm.setType (_cmt);
            if (_cmt == ColourMapType::Monochrome) { this->cm.setHue (_hue); }
            if (_cmt == ColourMapType::Fixed) { this->cm.setHSV (_hue, _sat, _val); }

            this->cm_sph.setType (_cmt_sph);
            if (_cmt_sph == ColourMapType::Monochrome) { this->cm_sph.setHue (_hue_sph); }
            if (_cmt_sph == ColourMapType::Fixed) { this->cm_sph.setHSV (_hue_sph, _sat_sph, _val_sph); }

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Convert datum using our scale into a colour triplet (RGB).
        std::array<float, 3> datumToColour (Flt datum_in) {
            // Scale the input...
            Flt datum = datum_in * this->scale[0] + this->scale[1];
            datum = datum > static_cast<Flt>(1.0) ? static_cast<Flt>(1.0) : datum;
            datum = datum < static_cast<Flt>(0.0) ? static_cast<Flt>(0.0) : datum;
            // ...then turn it into a colour:
            return this->cm.convert (datum);
        }

        //! Do the computations to initialize the vertices that will represent the surface.
        void initializeVertices (void) {

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
            VBOint ib = 0; // only 16 bits, 65536 vertices. Not enough!

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
            while (r2 != prlen) { // While through all 'rows' - pairs of pointrows

                this->computeSphere (ib, (*this->dataCoords)[r1], this->cm_sph.convert(dcopy[r1]), this->sradius, this->srings, this->sseg);
                this->computeSphere (ib, (*this->dataCoords)[r2], this->cm_sph.convert(dcopy[r2]), this->sradius, this->srings, this->sseg);
                this->computeTube (ib, (*this->dataCoords)[r1], (*this->dataCoords)[r2],
                                   this->cm.convert(dcopy[r1]), this->cm.convert(dcopy[r2]), this->radius, this->tseg);
                // Now while through the row pushing the rest of the vertices.
                while (r2 <= r2_e && r1 <= r1_e) {

                    // Now iterate r1 and r2 until we get to the end of the two rows.
                    // vtx is r1, r2. Question is: Is other vertex ++r1 or ++r2?
                    size_t r1n = r1+1;
                    size_t r2n = r2+1;

                    // Increment and make sure we didn't drop off the end
                    if (r1n > r1_e && r2n > r2_e) {
                        r1 = r1n;
                        r2 = r2n;
                        break;
                    }

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
                        float alpha1 = std::acos ((bsq + csq - asq)/(2*b*c));

                        bsq = r2_to_r2n_sq;
                        b = r2_to_r2n;
                        csq = r1_to_r2n_sq;
                        c = r1_to_r2n;
                        float alpha2 = std::acos ((bsq + csq - asq)/(2*b*c));

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
                        this->computeSphere (ib, (*this->dataCoords)[r1], this->cm_sph.convert(dcopy[r1]), this->sradius, this->srings, this->sseg);
                        this->computeSphere (ib, (*this->dataCoords)[r1n], this->cm_sph.convert(dcopy[r1n]), this->sradius, this->srings, this->sseg);
                        this->computeTube (ib, (*this->dataCoords)[r1], (*this->dataCoords)[r1n],
                                           this->cm.convert(dcopy[r1]), this->cm.convert(dcopy[r1n]), this->radius, this->tseg);
                        r1 = r1n;
                    } else {
                        // r2 is next
                        this->computeSphere (ib, (*this->dataCoords)[r2], this->cm_sph.convert(dcopy[r2]), this->sradius, this->srings, this->sseg);
                        this->computeSphere (ib, (*this->dataCoords)[r2n], this->cm_sph.convert(dcopy[r2n]), this->sradius, this->srings, this->sseg);
                        this->computeTube (ib, (*this->dataCoords)[r2], (*this->dataCoords)[r2n],
                                           this->cm.convert(dcopy[r2]), this->cm.convert(dcopy[r2n]), this->radius, this->tseg);
                        r2 = r2n;
                    }

                    if (completed_end_tri == true) { break; }

                    // Next tri:
                    this->computeSphere (ib, (*this->dataCoords)[r1], this->cm_sph.convert(dcopy[r1]), this->sradius, this->srings, this->sseg);
                    this->computeSphere (ib, (*this->dataCoords)[r2], this->cm_sph.convert(dcopy[r2]), this->sradius, this->srings, this->sseg);
                    this->computeTube (ib, (*this->dataCoords)[r1], (*this->dataCoords)[r2],
                                       this->cm.convert(dcopy[r1]), this->cm.convert(dcopy[r2]), this->radius, this->tseg);
                }

                // On to the next rows:
                r1 = r1_e + 1;
                r1_e = r1;
                r2 = r2_e; ++r2;
                r2_e = r2;

                if (r2 == prlen) { break; }

                x = (*this->dataCoords)[r1][pa];
                while (r1_e != prlen && (*this->dataCoords)[r1_e][pa] == x) { ++r1_e; }
                r1_e--;
                r2_e = r2;
                x = (*this->dataCoords)[r2][pa];
                while (r2_e != prlen && (*this->dataCoords)[r2_e][pa] == x) { ++r2_e; }
                r2_e--;
                // Now r1, r1_e, r2 and r2_e all point to the right places
            }
            std::cout << "PointRowsMeshVisual has " << ib << " vertex indices\n";
        }

    private:
        //! Which axis are we perpendicular to?
        size_t pa = 0;
        //! tube radius
        float radius = 0.05f;
        //! sphere radius
        float sradius = 0.052f;
        //! sphere rings
        int srings = 10;
        //! sphere segments
        int sseg = 12;
        //! tube segments
        int tseg = 12;
        //! A colour map for the spheres
        morph::ColourMap<float> cm_sph;
    };

} // namespace morph
