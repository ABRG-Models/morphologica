#pragma once

#include "GL3/gl3.h"
#include "tools.h"
#include "VisualDataModel.h"
#include "MathAlgo.h"
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <array>
using std::array;
#include <algorithm>
using std::transform;
#include <functional>
using std::plus;
using std::minus;
using std::multiplies;

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
    template <class Flt>
    class QuiverVisual : public VisualDataModel<Flt>
    {
    public:
        QuiverVisual(GLuint sp,
                     vector<array<Flt,3>>* _coords,
                     const array<float, 3> _offset,
                     const vector<array<Flt,3>>* _quivers,
                     ColourMapType _cmt,
                     const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);

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
                cout << "ncoords != nquiv, return." << endl;
                return;
            }

            array<Flt, 3> zero3 = {0.0,0.0,0.0};
            vector<Flt> lengths;
            for (unsigned int i = 0; i < nquiv; ++i) {
                lengths.push_back (MathAlgo<Flt>::distance (zero3, (*this->vectorData)[i]));
            }
            // Auto scale the lengths to get a full range of colours for the lengths.
            vector<Flt> lengthcolours = MathAlgo<Flt>::autoscale (lengths);

            // The indices index
            GLushort idx = 0;

            array<Flt,3> half = {0.5,0.5,0.5};
            array<Flt, 3> start, end, coords_i, vectorData_i, halfquiv;
            array<Flt, 3> clr;
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords_i = (*this->dataCoords)[i];
                vectorData_i = (*this->vectorData)[i];
                clr = this->cm.convert (lengthcolours[i]);
                if (this->qgoes == QuiverGoes::FromCoord) {
                    start = coords_i;
                    // end = (*this->dataCoords)[i] + (*this->vectorData)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               vectorData_i.begin(), end.begin(), plus<Flt>());


                } else if (this->qgoes == QuiverGoes::ToCoord) {
                    // start = (*this->dataCoords)[i] - (*this->vectorData)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               vectorData_i.begin(), start.begin(), minus<Flt>());

                    end = coords_i;
                } else /* if (this->qgoes == QuiverGoes::OnCoord) */ {
                    transform (half.begin(), half.end(),
                               vectorData_i.begin(), halfquiv.begin(), multiplies<Flt>());
                    //start = (*this->dataCoords)[i] - 0.5 * (*this->vectorData)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               halfquiv.begin(), start.begin(), minus<Flt>());
                    //end = (*this->dataCoords)[i] + 0.5 * (*this->vectorData)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               halfquiv.begin(), end.begin(), plus<Flt>());
                }
                // Will need a fixed scale for some visualizations
                this->computeTube (idx, start, end, clr, clr, lengths[i]/30.0);
                // Plus sphere or cone:
                this->computeSphere (idx, coords_i, clr, lengths[i]/10.0);
                // Compute a tip for the cone.
                array<Flt, 3> frac = {0.2,0.2,0.2};
                array<Flt, 3> tip;
                // Multiply vectorData_i by a fraction and that's the cone end. Note reuse of halfquiv variable
                transform (frac.begin(), frac.end(), vectorData_i.begin(), halfquiv.begin(), multiplies<Flt>());
                transform (end.begin(), end.end(), halfquiv.begin(), tip.begin(), plus<Flt>());
                this->computeCone (idx, end, tip, -0.1f, clr, lengths[i]/10.0);
            }
        }

        //! An enumerated type to say whether we draw quivers with coord at mid point;
        //! start point or end point
        QuiverGoes qgoes = QuiverGoes::FromCoord;
    };

} // namespace morph
