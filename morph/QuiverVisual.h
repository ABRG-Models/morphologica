#pragma once

#include <morph/tools.h>
#include <morph/VisualDataModel.h>
#include <morph/MathAlgo.h>
#include <morph/scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/colour.h>
#include <morph/graphstyles.h>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <functional>

namespace morph {

    //! A class to make quiver plots
    template <typename Flt, int glver = morph::gl::version_4_1>
    class QuiverVisual : public VisualDataModel<Flt, glver>
    {
    public:
        QuiverVisual(std::vector<vec<float>>* _coords,
                     const vec<float> _offset,
                     const std::vector<vec<Flt,3>>* _quivers,
                     ColourMapType _cmt,
                     const float _hue = 0.0f) {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->dataCoords = _coords;
            this->vectorData = _quivers;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->length_scale.do_autoscale = true;
        }

        // Call before initializeVertices() to scale quiver lengths logarithmically
        void setlog() { this->length_scale.setlog(); }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords->size();
            unsigned int nquiv = this->vectorData->size();

            if (ncoords != nquiv) {
                std::cout << "ncoords != nquiv, return." << std::endl;
                return;
            }

            vvec<Flt> dlengths;
            // Compute the lengths of each vector
            for (unsigned int i = 0; i < nquiv; ++i) {
                dlengths.push_back ( (*this->vectorData)[i].length() );
            }

            // Linearly scale the dlengths to generate colours
            vvec<Flt> lengthcolours(dlengths);

            // Make sure we can do an autoscale if the scaling was not already set
            if (!this->colourScale.ready()) { this->colourScale.do_autoscale = true; }

            // Set the colours based on either length of the vectors or values in
            // this->scalarData:
            if (this->scalarData == nullptr || this->scalarData->size() == 0) {
                if (this->do_quiver_length_scaling == false) {
                    // lengthcolours already is a copy of dlengths, but given that in this
                    // case we're NOT scaling, we have to make sure that the colours are all
                    // usable, so set to 0.5.
                    lengthcolours.set_from (Flt{0.5});
                } else  {
                    this->colourScale.transform (dlengths, lengthcolours);
                }
            } else {
                // We have scalarData, use these for the colours
                vvec<Flt> sdata (this->scalarData->size());
                std::copy (this->scalarData->begin(), this->scalarData->end(), sdata.begin());
                this->colourScale.transform  (sdata, lengthcolours);
            }

            // Now scale the lengths for their size on screen. Do this with a linear or log scaling.

            // (if log) First replace zeros with NaNs so that log transform will work.
            if (this->do_quiver_length_scaling == true && this->length_scale.getType() == morph::scaling_function::Logarithmic) {
                dlengths.search_replace (Flt{0}, std::numeric_limits<Flt>::quiet_NaN());
            }

            // Transform data lengths into "nrmlzedlengths"
            vvec<float> nrmlzedlengths (dlengths.size());
            std::copy (dlengths.begin(), dlengths.end(), nrmlzedlengths.begin());
            if (this->fixed_length != 0.0f) {
                nrmlzedlengths.set_from (this->fixed_length);
            } else if (this->do_quiver_length_scaling == true) {
                this->length_scale.transform (dlengths, nrmlzedlengths);
            } // else lengths are left as in the data

            // Find the scaling factor to scale real lengths into screen lengths, which are the
            // normalized lengths multiplied by a user-settable quiver_length_gain.
            vvec<float> lfactor = nrmlzedlengths/dlengths * this->quiver_length_gain;

            vec<Flt> half = { Flt{0.5}, Flt{0.5}, Flt{0.5} };
            vec<Flt> vectorData_i, halfquiv;
            vec<float> start, end, coords_i;
            std::array<float, 3> clr;
            for (unsigned int i = 0; i < ncoords; ++i) {

                coords_i = (*this->dataCoords)[i];

                float len = nrmlzedlengths[i] * this->quiver_length_gain;
                if ((std::isnan(dlengths[i]) || dlengths[i] == Flt{0}) && this->show_zero_vectors) {
                    // NaNs denote zero vectors when the lengths have been log scaled.
                    this->computeSphere (coords_i, zero_vector_colour, this->zero_vector_marker_size * quiver_thickness_gain);
                    continue;
                }

                vectorData_i = (*this->vectorData)[i];
                vectorData_i *= lfactor[i];

                clr = this->cm.convert (lengthcolours[i]);

                if (this->qgoes == QuiverGoes::FromCoord) {
                    start = coords_i;
                    std::transform (coords_i.begin(), coords_i.end(), vectorData_i.begin(), end.begin(), std::plus<Flt>());

                } else if (this->qgoes == QuiverGoes::ToCoord) {
                    std::transform (coords_i.begin(), coords_i.end(), vectorData_i.begin(), start.begin(), std::minus<Flt>());
                    end = coords_i;
                } else /* if (this->qgoes == QuiverGoes::OnCoord) */ {
                    std::transform (half.begin(), half.end(), vectorData_i.begin(), halfquiv.begin(), std::multiplies<Flt>());
                    std::transform (coords_i.begin(), coords_i.end(), halfquiv.begin(), start.begin(), std::minus<Flt>());
                    std::transform (coords_i.begin(), coords_i.end(), halfquiv.begin(), end.begin(), std::plus<Flt>());
                }

                // How thick to draw the quiver arrows? Can scale by length (default) or keep
                // constant (set fixed_quiver_thickness > 0)
                float quiv_thick = this->fixed_quiver_thickness ? this->fixed_quiver_thickness : len*quiver_thickness_gain;

                // The right way to draw an arrow.
                vec<float> arrow_line = end - start;
                vec<float> cone_start = arrow_line.shorten (len*quiver_arrowhead_prop);
                cone_start += start;
                this->computeTube (start, cone_start, clr, clr, quiv_thick, shapesides);
                float conelen = (end-cone_start).length();
                if (arrow_line.length() > conelen) {
                    this->computeCone (cone_start, end, 0.0f, clr, quiv_thick*2.0f, shapesides);
                }

                if (this->show_coordinate_sphere == true) {
                    // Draw a sphere on the coordinate:
                    this->computeSphere (coords_i, clr, quiv_thick*2.0f, shapesides/2, shapesides);
                }
            }
        }

        //! An enumerated type to say whether we draw quivers with coord at mid point; start point or end point
        QuiverGoes qgoes = QuiverGoes::FromCoord;

        // How many sides to an arrow/cone/sphere? Increase for smoother arrow
        // objects. Decrease to ease the load on your CPU and GPU. 12 is a reasonable
        // compromise. You can set this before calling finalize().
        int shapesides = 12;

        // Setting a fixed length can be useful to focus on the flow of the field.
        Flt fixed_length = 0.0f;

        // Allows user to linearly scale the size of the quivers that are plotted. Set before
        // calling finalize()
        float quiver_length_gain = 1.0f;

        // If 0, then quiver thickness is scaled by quiver length. Otherwise, the quiver arrowshaft
        // tubes have radius = fixed_quiver_thickness * quiver_thickness_gain. Try small values like 0.01f.
        float fixed_quiver_thickness = 0.0f;

        // Allows user to scale the thickness of the quivers.
        float quiver_thickness_gain = 0.05f;

        // What proportion of the arrow length should the arrowhead length be?
        float quiver_arrowhead_prop = 0.25f;

        // If true, show a marker indicating the location of zero vectors
        bool show_zero_vectors = false;

        // If false then omit the sphere drawn on the coordinate location
        bool show_coordinate_sphere = true;

        // User can choose a colour
        std::array<float, 3> zero_vector_colour = morph::colour::crimson;

        // User can choose size of zero vector markers (which are spheres)
        float zero_vector_marker_size = 0.05f;

        // The input vectors are scaled in length to the range [0, 1], which is then modified by the
        // user using quiver_length_gain. This scaling can be made logarithmic by calling
        // QuiverVisual::setlog() before calling finalize(). The scaling can be ignored by calling
        // QuiverVisual::length_scale.compute_scaling (0, 1); before finalize().
        morph::scale<Flt, float> length_scale;
        // Set this false to avoid applying length_scale to quiver lengths and also and
        // colourScale (in the absence of ScalarData).
        bool do_quiver_length_scaling = true;
    };

} // namespace morph
