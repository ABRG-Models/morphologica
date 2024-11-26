#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/ColourMap.h>
#include <array>

namespace morph {

    /*!
     * This class creates the vertices for an geodesic polyhedron in a 3D scene using
     * the constexpr function.
     *
     * \tparam T the type for the data to be visualized as face (or maybe vertex) colours
     *
     * \tparam glver The usual OpenGL version code; match this to everything else in
     * your program.
     */
    template<typename T, int iterations, int glver = morph::gl::version_4_1>
    class GeodesicVisualCE : public VisualModel<glver>
    {
    public:
        GeodesicVisualCE() { this->init ({0.0, 0.0, 0.0}, 1.0f); }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        GeodesicVisualCE(const vec<float, 3> _offset, const float _radius)
        {
            this->init (_offset, _radius);
        }

        ~GeodesicVisualCE () {}

        void init (const vec<float, 3> _offset, const float _radius)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->radius = _radius;
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            constexpr morph::vec<float, 3> zvec  = {0,0,0};
            if (iterations > 5) {
                // Note odd necessity to stick in the 'template' keyword after this->
                this->template computeSphereGeoFast<double, iterations> (zvec, this->colour, this->radius);
            } else {
                // computeSphereGeo F defaults to float
                this->template computeSphereGeoFast<float, iterations> (zvec, this->colour, this->radius);
            }
        }

        //! The radius of the geodesic
        float radius = 1.0f;
        //! Fixed colour.
        std::array<float, 3> colour = { 0.2f, 0.1f, 0.7f };
    };

} // namespace morph
