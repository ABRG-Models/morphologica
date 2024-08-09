#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for a cylindrical 'rod' in a 3D scene.
    template<int glver = morph::gl::version_4_1>
    class RodVisual : public VisualModel<glver>
    {
    public:
        RodVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        RodVisual(const vec<float, 3> _offset,
                  const vec<float, 3> _start_coord, const vec<float, 3> _end_coord, const float _radius,
                  const std::array<float, 3> _col)
        {
            this->init (_offset, _start_coord, _end_coord, _radius, _col, _col);
        }

        //! Initialise with offset, start and end coordinates, radius and start and end colours.
        RodVisual(const vec<float, 3> _offset,
                  const vec<float, 3> _start_coord, const vec<float, 3> _end_coord, const float _radius,
                  const std::array<float, 3> _start_col, const std::array<float, 3> _end_col)
        {
            this->init (_offset, _start_coord, _end_coord, _radius, _start_col, _end_col);
        }

        ~RodVisual () {}

        void init (const vec<float, 3> _offset,
                   const vec<float, 3> _start_coord, const vec<float, 3> _end_coord, const float _radius,
                   const std::array<float, 3> _start_col, const std::array<float, 3> _end_col)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);

            this->start_coord = _start_coord;
            this->end_coord = _end_coord;
            this->radius = _radius;
            this->start_col = _start_col;
            this->end_col = _end_col;
        }

        static constexpr bool use_oriented_tube = false;
        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            // Draw a tube. That's it!
            if constexpr (use_oriented_tube == false) {
                this->computeTube (this->start_coord, this->end_coord,
                                   this->start_col, this->end_col, this->radius, 12);
            } else {
                // Can alternatively use the 'oriented' tube
                this->computeTube (this->start_coord, this->end_coord,
                                   {0,1,0}, {0,0,1},
                                   this->start_col, this->end_col, this->radius, 6, morph::mathconst<float>::pi_over_6);
            }
        }

        //! The position of the start of the rod, given with respect to the parent's offset
        vec<float, 3> start_coord = {0.0f, 0.0f, 0.0f};
        //! The position of the end of the rod, given with respect to the parent's offset
        vec<float, 3> end_coord = {1.0f, 0.0f, 0.0f};
        //! The radius of the rod
        float radius = 1.0f;

        //! The colours of the rod.
        std::array<float, 3> start_col = {1.0f, 0.0f, 0.0f};
        std::array<float, 3> end_col = {0.0f, 0.0f, 1.0f};
    };

} // namespace morph
