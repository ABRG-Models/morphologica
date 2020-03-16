#pragma once

#include "GL3/gl3.h"
#include "GL/glext.h"

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

#include <algorithm>
using std::transform;
#include <functional>
using std::plus;
using std::minus;
using std::multiplies;

namespace morph {

    //! How does a quiver go? Does it start at teh coordinate (it goes from the
    //! coordinate; FromCoord) or does it sit on top of the coord
    //! (OnCoord). Alternatively, it could go to the coordinate; ToCoord.
    enum class QuiverGoes {
        FromCoord,
        ToCoord,
        OnCoord
    };

    //! A class to make quiver plots
    template <class Flt>
    class QuiverVisual : public VisualModel
    {
    public:
        QuiverVisual(GLuint sp,
                     const vector<array<Flt,3>>* _coords,
                     const array<float, 3> _offset,
                     const vector<array<Flt,3>>* _quivers,
                     ColourMapType _cmt,
                     const float _hue = 0.0f) {
            // Set up...
            this->shaderprog = sp;
            this->offset = _offset;
            this->viewmatrix.translate (this->offset);
            this->coords = _coords;
            this->quivers = _quivers;

            this->cm.setHue (_hue);
            this->cm.setType (_cmt);

            this->initializeVertices();
            this->postVertexInit();
        }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices (void) {

            unsigned int ncoords = this->coords->size();
            unsigned int nquiv = this->quivers->size();

            if (ncoords != nquiv) {
                cout << "ncoords != nquiv, return." << endl;
                return;
            }

            array<Flt, 3> zero3 = {0.0,0.0,0.0};
            vector<Flt> lengths;
            for (unsigned int i = 0; i < nquiv; ++i) {
                lengths.push_back (MathAlgo<Flt>::distance (zero3, (*this->quivers)[i]));
            }
            // Auto scale the lengths to get a full range of colours for the lengths.
            vector<Flt> lengthcolours = MathAlgo<Flt>::autoscale (lengths);

            // The indices index
            GLushort idx = 0;

            array<Flt,3> half = {0.5,0.5,0.5};
            array<Flt, 3> start, end, coords_i, quivers_i, halfquiv;
            array<Flt, 3> clr;
            for (unsigned int i = 0; i < ncoords; ++i) {
                coords_i = (*this->coords)[i];
                quivers_i = (*this->quivers)[i];
                clr = this->cm.convert (lengthcolours[i]);
                if (this->qgoes == QuiverGoes::FromCoord) {
                    start = coords_i;
                    // end = (*this->coords)[i] + (*this->quivers)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               quivers_i.begin(), end.begin(), plus<Flt>());


                } else if (this->qgoes == QuiverGoes::ToCoord) {
                    // start = (*this->coords)[i] - (*this->quivers)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               quivers_i.begin(), start.begin(), minus<Flt>());

                    end = coords_i;
                } else /* if (this->qgoes == QuiverGoes::OnCoord) */ {
                    transform (half.begin(), half.end(),
                               quivers_i.begin(), halfquiv.begin(), multiplies<Flt>());
                    //start = (*this->coords)[i] - 0.5 * (*this->quivers)[i];
                    transform (coords_i.begin(), coords_i.end(),
                               halfquiv.begin(), start.begin(), minus<Flt>());
                    //end = (*this->coords)[i] + 0.5 * (*this->quivers)[i];
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
                // Multiply quivers_i by a fraction and that's the cone end. Note reuse of halfquiv variable
                transform (frac.begin(), frac.end(), quivers_i.begin(), halfquiv.begin(), multiplies<Flt>());
                transform (end.begin(), end.end(), halfquiv.begin(), tip.begin(), plus<Flt>());
                this->computeCone (idx, end, tip, -0.1f, clr, lengths[i]/10.0);
            }
        }

        //! Update the data and re-compute the vertices.
        //@{
        void updateData (const vector<array<Flt, 3>>* _quivers) {
            this->quivers = _quivers;
            this->reinit();
        }
        void updateData (const vector<array<Flt, 3>>* _coords, const vector<array<Flt, 3>>* _quivers) {
            this->coords = _coords;
            this->quivers = _quivers;
            this->reinit();
        }
        //@}

        void reinit (void) {
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

        //! A colour map to colour the arrows. Could colour based on length, say.
        ColourMap<Flt> cm;

        //! An enumerated type to say whether we draw quivers with coord at mid point;
        //! start point or end point
        QuiverGoes qgoes = QuiverGoes::FromCoord;

    private:

        //! The coordinates at which to draw the quivers.
        const vector<array<Flt,3>>* coords;

        //! The vectors to visualize at each coordinate.
        const vector<array<Flt,3>>* quivers;
    };

} // namespace morph
