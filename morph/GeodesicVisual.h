#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <array>

namespace morph {

    //! This class creates the vertices for an geodesic polyhedron in a 3D scene.
    template<int glver = morph::gl::version_4_1>
    class GeodesicVisual : public VisualModel<glver>
    {
    public:
        GeodesicVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        GeodesicVisual(const vec<float, 3> _offset,
                       const float _radius,
                       const std::array<float, 3> _col)
        {
            this->init (_offset, _radius, _col);
        }

        ~GeodesicVisual () {}

        void init (const vec<float, 3> _offset,
                   const float _radius,
                   const std::array<float, 3> _col)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->radius = _radius;
            this->colour = _col;
        }

        static constexpr bool use_oriented_tube = false;
        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            if (iterations > 5) {
                // Note odd necessity to stick in the 'template' keyword after this->
                this->n_faces = this->template computeSphereGeo<double> (this->idx, morph::vec<float, 3>({0,0,0}),
                                                                         this->colour, this->radius, this->iterations);
            } else {
                // computeSphereGeo F defaults to float
                this->n_faces = this->computeSphereGeo (this->idx, morph::vec<float, 3>({0,0,0}),
                                                        this->colour, this->radius, this->iterations);
            }
        }

        //! The radius of the geodesic
        float radius = 1.0f;
        //! The colour of the object
        std::array<float, 3> colour = {0, 1, 0.3};
        //! The number of iterations in the geodesic sphere. Set before finalize() to change from the default.
        int iterations = 2;
        //! This will be filled with the number of faces in the geodesic
        int n_faces = 0;
    };

} // namespace morph
