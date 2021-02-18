/*!
 * \file
 *
 * Visualise a bunch of agents (as spheres), each of which has a history of locations
 * that it has visited previously, which could be shown as tracks (tubes, say).
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
#include <vector>
#include <array>

#include "branch.h"

template <typename Flt>
class BranchVisual : public morph::VisualModel
{
public:
    BranchVisual(GLuint sp, const morph::Vector<float, 3> _offset, std::vector<branch<Flt>>* _branches)
    {
        this->branches = _branches;
        this->shaderprog = sp;
        this->mv_offset = _offset;
        this->viewmatrix.translate (this->mv_offset);
    }

    //! Compute spheres for a scatter plot
    void initializeVertices (void)
    {
        VBOint idx = 0;
        // For each branch, draw it.
        for (auto b : *this->branches) {
            // Colour comes from target location.
            std::array<float, 3> clr = { b.tz[0], b.tz[1], 0 };
            morph::Vector<float, 3> last = { 0, 0, 0 };
            morph::Vector<float, 3> cur = { 0, 0, 0 };
            // First draw the path
            for (unsigned int i = 1; i < b.path.size(); ++i) {
                last[0] = b.path[i-1][0];
                last[1] = b.path[i-1][1];
                cur[0] = b.path[i][0];
                cur[1] = b.path[i][1];
                this->computeFlatLineRnd (idx, last, cur, this->uz, clr, this->linewidth, 0.0f, true, false);
            }
            // Finally, a sphere at the last location
            this->computeSphere (idx, cur, clr, this->radiusFixed, 10, 12);
        }
    }

    //! Set this->radiusFixed, then re-compute vertices.
    void setRadius (float fr)
    {
        this->radiusFixed = fr;
        this->reinit();
    }

    //! Pointer to a vector of branches to visualise
    std::vector<branch<Flt>>* branches = (std::vector<branch<Flt>>*)0;
    //! Change this to get larger or smaller spheres.
    Flt radiusFixed = 0.02;
    Flt linewidth = 0.02;
    //! A normal vector, fixed as pointing up
    morph::Vector<float, 3> uz = {0,0,1};
    // Hues for colour control with vectorData
    float hue1 = 0.1f;
    float hue2 = 0.5f;
    float hue3 = -1.0f;
};
