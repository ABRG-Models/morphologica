/*!
 * \file
 *
 * Visualise a network of locations on a rectangular grid, with lines to their intended
 * neighbours to the north, south, east and west. Intended to reproduce the
 * visualisations in the Simpson and Goodhill paper.
 *
 * \author Seb James
 * \date 2021
 */
#pragma once

#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#include <morph/VisualModel.h>
#include <morph/Vector.h>
#include <morph/Scale.h>
#include <vector>
#include <array>

#include "net.h"

template <typename Flt>
class NetVisual : public morph::VisualModel
{
public:
    NetVisual(GLuint sp, const morph::Vector<float, 3> _offset, net<Flt>* _locations)
    {
        this->locations = _locations;
        this->shaderprog = sp;
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    void initializeVertices (void)
    {
        VBOint idx = 0;
        std::array<float, 3> clr = { 0, 0, 0 };
        // Spheres at the net vertices
        for (auto p : this->locations->p) {
            this->computeSphere (idx, p, clr, this->radiusFixed, 14, 12);
        }
        // Connections
        for (auto c : this->locations->c) {
            morph::Vector<Flt, 3> c1 = this->locations->p[c[0]];
            morph::Vector<Flt, 3> c2 = this->locations->p[c[1]];
            this->computeFlatLineRnd (idx, c1, c2, this->uz, clr, this->linewidth, 0.0f, true, false);
        }
    }

    //! Set this->radiusFixed, then re-compute vertices.
    void setRadius (float fr)
    {
        this->radiusFixed = fr;
        this->reinit();
    }

    //! Pointer to a vector of locations to visualise
    net<Flt>* locations = (net<Flt>*)0;
    Flt radiusFixed = 0.01;
    Flt linewidth = 0.008;
    //! A normal vector, fixed as pointing up
    morph::Vector<float, 3> uz = {0,0,1};
    // Hues for colour control with vectorData
    //float hue1 = 0.1f;
    //float hue2 = 0.5f;
    //float hue3 = -1.0f;
};
