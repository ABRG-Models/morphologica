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
        QuiverVisual(GLuint sp,
                     std::vector<vec<float>>* _coords,
                     const vec<float> _offset,
                     const std::vector<vec<Flt,3>>* _quivers,
                     ColourMapType _cmt,
                     const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->dataCoords = _coords;
            this->vectorData = _quivers;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices (void) {

            unsigned int ncoords = this->dataCoords->size();
            unsigned int nquiv = this->vectorData->size();

            if (ncoords != nquiv) {
                std::cout << "ncoords != nquiv, return." << std::endl;
                return;
            }

            vec<Flt> zero3 = {0.0,0.0,0.0};
            std::vector<Flt> lengths;
            for (unsigned int i = 0; i < nquiv; ++i) {
                lengths.push_back (MathAlgo::distance<Flt, 3> (zero3, (*this->vectorData)[i]));
            }
            // Auto scale the lengths to get a full range of colours for the lengths.
            std::vector<Flt> lengthcolours = MathAlgo::autoscale (lengths, 0.0f, 1.0f);

            // The indices index
            VBOint idx = 0;

            vec<Flt> half = {0.5,0.5,0.5};
            vec<Flt> vectorData_i, halfquiv;
            vec<float> start, end, coords_i;
            std::array<float, 3> clr;
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords_i = (*this->dataCoords)[i];
                vectorData_i = (*this->vectorData)[i];
                clr = this->cm.convert (lengthcolours[i]);
                if (this->qgoes == QuiverGoes::FromCoord) {
                    start = coords_i;
                    // end = (*this->dataCoords)[i] + (*this->vectorData)[i];
                    std::transform (coords_i.begin(), coords_i.end(),
                                    vectorData_i.begin(), end.begin(), std::plus<Flt>());


                } else if (this->qgoes == QuiverGoes::ToCoord) {
                    // start = (*this->dataCoords)[i] - (*this->vectorData)[i];
                    std::transform (coords_i.begin(), coords_i.end(),
                                    vectorData_i.begin(), start.begin(), std::minus<Flt>());

                    end = coords_i;
                } else /* if (this->qgoes == QuiverGoes::OnCoord) */ {
                    std::transform (half.begin(), half.end(),
                                    vectorData_i.begin(), halfquiv.begin(), std::multiplies<Flt>());
                    //start = (*this->dataCoords)[i] - 0.5 * (*this->vectorData)[i];
                    std::transform (coords_i.begin(), coords_i.end(),
                                    halfquiv.begin(), start.begin(), std::minus<Flt>());
                    //end = (*this->dataCoords)[i] + 0.5 * (*this->vectorData)[i];
                    std::transform (coords_i.begin(), coords_i.end(),
                                    halfquiv.begin(), end.begin(), std::plus<Flt>());
                }
                // Will need a fixed scale for some visualizations
                this->computeTube (idx, start, end, clr, clr, lengths[i]/30.0);
                // Plus sphere or cone:
                this->computeSphere (idx, coords_i, clr, lengths[i]/10.0);
                // Compute a tip for the cone.
                vec<Flt> frac = { Flt{0.2}, Flt{0.2}, Flt{0.2} };
                vec<float> tip;
                // Multiply vectorData_i by a fraction and that's the cone end. Note
                // reuse of halfquiv variable
                std::transform (frac.begin(), frac.end(),
                                vectorData_i.begin(), halfquiv.begin(), std::multiplies<Flt>());
                std::transform (end.begin(), end.end(),
                                halfquiv.begin(), tip.begin(), std::plus<Flt>());
                this->computeCone (idx, end, tip, -0.1f, clr, lengths[i]/10.0);
            }
        }

        //! An enumerated type to say whether we draw quivers with coord at mid point;
        //! start point or end point
        QuiverGoes qgoes = QuiverGoes::FromCoord;
    };

} // namespace morph
