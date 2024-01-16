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

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <morph/VisualModel.h>
#include <morph/vec.h>
#include <morph/Scale.h>
#include <array>

#include "net.h"

template <typename Flt>
class NetVisual : public morph::VisualModel<>
{
public:
    NetVisual(const morph::vec<float, 3> _offset, net<Flt>* _locations)
    {
        this->locations = _locations;
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    void initializeVertices()
    {
        // Spheres at the net vertices
        for (unsigned int i = 0; i < this->locations->p.size(); ++i) {
            this->computeSphere (this->idx, this->locations->p[i], this->locations->clr[i], this->radiusFixed, 14, 12);
        }
        // Connections
        for (auto c : this->locations->c) {
            morph::vec<Flt, 3> c1 = this->locations->p[c[0]];
            morph::vec<Flt, 3> c2 = this->locations->p[c[1]];
            std::array<float, 3> clr1 = this->locations->clr[c[0]];
            std::array<float, 3> clr2 = this->locations->clr[c[1]];
            this->computeLine (this->idx, c1, c2, this->uz, clr1, clr2, this->linewidth, this->linewidth/Flt{4});
        }
    }

    //! Set this->radiusFixed, then re-compute vertices.
    void setRadius (float fr)
    {
        this->radiusFixed = fr;
        this->reinit();
    }

    //! Pointer to a vector of locations to visualise
    net<Flt>* locations = nullptr;
    Flt radiusFixed = Flt{0.01};
    Flt linewidth = Flt{0.008};
    //! A normal vector, fixed as pointing up
    morph::vec<float, 3> uz = {0.0f, 0.0f, 1.0f};
};
