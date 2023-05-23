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
#include <algorithm>
#include <functional>

namespace morph {

    //! How does a quiver go? Does it start at the coordinate? If so, it goes 'from'
    //! the coordinate; FromCoord. Does it instead sit on top of the coord (OnCoord)?
    //! Alternatively, it could go 'to' the coordinate; ToCoord.
    enum class QuiverGoes {
        FromCoord,
        ToCoord,
        OnCoord
    };

    //! A class to make quiver plots
    template <typename Flt>
    class QuiverVisual : public VisualDataModel<Flt>
    {
    public:
        QuiverVisual(morph::gl::shaderprogs& _shaders,
                     std::vector<vec<float>>* _coords,
                     const vec<float> _offset,
                     const std::vector<vec<Flt,3>>* _quivers,
                     ColourMapType _cmt,
                     const float _hue = 0.0f) {
            // Set up...
            this->shaders = _shaders;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->dataCoords = _coords;
            this->vectorData = _quivers;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->length_scale.do_autoscale = true;
            //this->length_scale.setlog();

        }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords->size();
            unsigned int nquiv = this->vectorData->size();

            if (ncoords != nquiv) {
                std::cout << "ncoords != nquiv, return." << std::endl;
                return;
            }

            vec<Flt> zero3 = { Flt{0}, Flt{0}, Flt{0} };
            std::vector<Flt> dlengths;
            for (unsigned int i = 0; i < nquiv; ++i) {
                dlengths.push_back (MathAlgo::distance<Flt, 3> (zero3, (*this->vectorData)[i]));
            }

            // Scale the lengths for their size on screen
            std::vector<float> lengths (dlengths.size());
            this->length_scale.compute_autoscale (0.00001, 0.2);
            this->length_scale.transform (dlengths, lengths);

            // Auto scale the lengths to get a full range of colours for the lengths.
            std::vector<Flt> lengthcolours = MathAlgo::autoscale (lengths, Flt{0}, Flt{1});

            vec<Flt> half = { Flt{0.5}, Flt{0.5}, Flt{0.5} };
            vec<Flt> vectorData_i, halfquiv;
            vec<float> start, end, coords_i;
            std::array<float, 3> clr;
            for (unsigned int i = 0; i < ncoords; ++i) {

                // If we want fixed length vector arrows, fixed_length should be set > 0.
                float len = this->fixed_length > 0.0f ? this->fixed_length : lengths[i];

                // For a fixed length vector, we'll have to scale each of vectorData
                float vmult = (this->fixed_length > 0.0f && lengths[i] > Flt{0}) ? (this->fixed_length/static_cast<float>(dlengths[i])) : 1.0f;
                coords_i = (*this->dataCoords)[i];
                vectorData_i = (*this->vectorData)[i];
                vectorData_i *= vmult; // apply the scaling (vmult may well be 1)
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
                // Will need a fixed scale for some visualizations
                this->computeTube (this->idx, start, end, clr, clr, len/30.0f);
                // Plus sphere or cone:
                this->computeSphere (this->idx, coords_i, clr, len/10.0f);
                // Compute a tip for the cone.
                vec<Flt> frac = { Flt{0.2}, Flt{0.2}, Flt{0.2} };
                vec<float> tip;
                // Multiply vectorData_i by a fraction and that's the cone end. Note reuse of halfquiv variable
                std::transform (frac.begin(), frac.end(), vectorData_i.begin(), halfquiv.begin(), std::multiplies<Flt>());
                std::transform (end.begin(), end.end(), halfquiv.begin(), tip.begin(), std::plus<Flt>());
                this->computeCone (this->idx, end, tip, -0.1f, clr, len/10.0f);
            }
        }

        //! An enumerated type to say whether we draw quivers with coord at mid point; start point or end point
        QuiverGoes qgoes = QuiverGoes::FromCoord;

        // Setting a fixed length can be useful to focus on the flow of the field.
        Flt fixed_length = 0.0f;

        // Should the length be scaled?
        morph::Scale<Flt, float> length_scale;
    };

} // namespace morph
