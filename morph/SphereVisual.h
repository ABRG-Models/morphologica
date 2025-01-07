/*
 * You just want a sphere visual model? Here it is.
 */
#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a simple sphere in a 3D scene.
    template<int glver = morph::gl::version_4_1>
    class SphereVisual : public VisualModel<glver>
    {
    public:
        SphereVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

        //! Initialise with offset, radius and a single colour.
        SphereVisual(const vec<float, 3> _offset, const float _radius, const std::array<float, 3> _col)
        {
            this->init (_offset, _radius, _col);
        }

        ~SphereVisual () {}

        void init (const vec<float, 3> _offset, const float _radius, const std::array<float, 3> _col)
        {
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->radius = _radius;
            this->sphere_colour = _col;
        }

        void initializeVertices()
        {
            this->computeSphere ({0,0,0}, this->sphere_colour, this->radius);
        }

        //! The radius of the sphere
        float radius = 1.0f;
        //! The colour of the sphere
        std::array<float, 3> sphere_colour = {1.0f, 0.0f, 0.0f};
    };

} // namespace morph
