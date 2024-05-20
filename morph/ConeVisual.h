#pragma once

/*!
 * \file Declares ConeVisual to visualize a simple cone
 */

#ifndef USE_GLEW
#ifdef __OSX__
# include <OpenGL/gl3.h>
#else
# include <GL3/gl3.h>
#endif
#endif
#include <array>
#include <morph/vec.h>
#include <morph/colour.h>

namespace morph {

    //! A class to visualize a single vector
    template <int glver = morph::gl::version_4_1>
    class ConeVisual : public VisualModel<glver>
    {
    public:
        ConeVisual(const vec<float> _offset)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
        }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices()
        {
            this->computeCone (this->idx, this->start, this->end, this->ringoffset, this->clr, this->radius, this->shapesides);
        }

        vec<float> clr = {1.0f, 0.0f, 0.7f};
        vec<float> start = {0,0,0};
        vec<float> end = {1,0,0};
        float radius = 0.3f;
        float ringoffset = 0.0f;

        // How many sides to an arrow/cone/sphere? Increase for smoother arrow
        // objects. Decrease to ease the load on your CPU and GPU. 12 is a reasonable
        // compromise. You can set this before calling finalize().
        int shapesides = 12;
    };

} // namespace morph
