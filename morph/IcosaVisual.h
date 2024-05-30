#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for an icosahedron in a 3D scene.
    template<int glver = morph::gl::version_4_1>
    class IcosaVisual : public VisualModel<glver>
    {
    public:
        IcosaVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        IcosaVisual(const vec<float, 3> _offset,
                    const float _radius,
                    const std::array<float, 3> _col)
        {
            this->init (_offset, _radius, _col);
        }

        ~IcosaVisual () {}

        void init (const vec<float, 3> _offset,
                   const float _radius,
                   const std::array<float, 3> _col)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->radius = _radius;
            for (auto& c : this->colours) { c = _col; }
        }

        static constexpr bool use_oriented_tube = false;
        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            this->computeIcosahedron (morph::vec<float, 3>({0,0,0}), this->colours, this->radius);
        }

        //! The radius of the icosahedrona
        float radius = 1.0f;
        //! The colours of the object
        std::array<std::array<float, 3>, 20> colours;
    };

} // namespace morph
