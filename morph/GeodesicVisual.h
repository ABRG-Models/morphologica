#pragma once

#include <morph/vec.h>
#include <morph/VisualModel.h>
#include <morph/mathconst.h>
#include <morph/Scale.h>
#include <morph/ColourMap.h>
#include <array>

namespace morph {

    /*!
     * This class creates the vertices for an geodesic polyhedron in a 3D scene.
     *
     * \tparam T the type for the data to be visualized as face (or maybe vertex) colours
     *
     * \tparam glver The usual OpenGL version code; match this to everything else in
     * your program.
     */
    template<typename T, int glver = morph::gl::version_4_1>
    class GeodesicVisual : public VisualModel<glver>
    {
    public:
        GeodesicVisual() { this->init ({0.0, 0.0, 0.0}, 1.0f); }

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        GeodesicVisual(const vec<float, 3> _offset, const float _radius)
        {
            this->init (_offset, _radius);
        }

        ~GeodesicVisual () {}

        void init (const vec<float, 3> _offset, const float _radius)
        {
            // Set up...
            this->mv_offset = _offset;
            this->viewmatrix.translate (this->mv_offset);
            this->radius = _radius;
            this->cm.setType (morph::ColourMapType::Jet);
            this->colourScale.do_autoscale = true;
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            morph::geometry::icosahedral_geodesic_info gi(this->iterations);
            this->n_faces = gi.n_faces;
            this->n_verts = gi.n_vertices;

            if (this->colourFaces == true) {
                // Resize our data.
                this->data.resize (this->n_faces, T{0});

                if (iterations > 5) {
                    this->n_verts = this->template computeSphereGeoFaces<double> (this->idx, morph::vec<float, 3>({0,0,0}),
                                                                                  this->cm.convert(0.0f), this->radius, this->iterations);
                } else {
                    this->n_verts = this->computeSphereGeoFaces (this->idx, morph::vec<float, 3>({0,0,0}),
                                                                 this->cm.convert(0.0f), this->radius, this->iterations);
                }

            } else { // colour vertices

                if (iterations > 5) {
                    // Note odd necessity to stick in the 'template' keyword after this->
                    this->n_verts = this->template computeSphereGeo<double> (this->idx, morph::vec<float, 3>({0,0,0}),
                                                                             this->cm.convert(0.0f), this->radius, this->iterations);
                } else {
                    // computeSphereGeo F defaults to float
                    this->n_verts = this->computeSphereGeo (this->idx, morph::vec<float, 3>({0,0,0}),
                                                            this->cm.convert(0.0f), this->radius, this->iterations);
                }
                // Resize our data.
                this->data.resize (n_verts, T{0});
            }
        }

        //! Update the colours based on vvec<T> data
        void updateColours()
        {
            if (this->data.empty() && this->cdata.empty()) { return; }

            size_t n_cvals = this->vertexColors.size();
            if (n_cvals == 0u) { return; } // model doesn't exist yet

            if (!this->cdata.empty()) {

                this->vertexColors.clear(); // could potentially just replace values
                size_t n_data = this->cdata.size();

                // there are n_vertex colours, and n_data data points.
                if (this->colourFaces == true) {
                    if (n_cvals != 3 * 3 * n_data) { throw std::runtime_error ("data is not right size");  }
                } else {
                    if (n_cvals != 3 * n_data) { throw std::runtime_error ("data is not right size");  }
                }

                // Re-colour
                for (size_t i = 0u; i < n_data; ++i) {
                    // Update the 3 RGB values in vertexColors
                    for (int ci = 0; ci < (this->colourFaces == true ? 3 : 1); ++ci) {
                        this->vertex_push (cdata[i], this->vertexColors);
                    }
                }

            } else {

                this->vertexColors.clear(); // could potentially just replace values
                size_t n_data = this->data.size();

                // there are n_vertex colours, and n_data data points.
                if (this->colourFaces == true) {
                    if (n_cvals != 3 * 3 * n_data) { throw std::runtime_error ("data is not right size");  }
                } else {
                    if (n_cvals != 3 * n_data) { throw std::runtime_error ("data is not right size");  }
                }

                // Scale data
                morph::vvec<float> scaled_data (this->data);
                if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
                this->colourScale.transform (this->data, scaled_data);
                //std::cout << "data range: " << data.range() << ", scaled_data range " << scaled_data.range() << std::endl;

                // Re-colour
                for (size_t i = 0u; i < n_data; ++i) {
                    // Update the 3 RGB values in vertexColors
                    auto c = this->cm.convert (scaled_data[i]);
                    for (int ci = 0; ci < (this->colourFaces == true ? 3 : 1); ++ci) {
                        this->vertex_push (c, this->vertexColors);
                    }
                }
            }
            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        //! The radius of the geodesic
        float radius = 1.0f;
        //! The colour of the object. Can be resized to n_faces to colour each face
        //! independently, or possibly to size of vertices to colour the vertices. Fill
        //! this vvec with data *after* calling initialize.
        morph::vvec<T> data;
        //! Can also colour with direct colour data
        morph::vvec<std::array<float, 3>> cdata;
        //! Do we colour vertices or faces? Set before finalize()
        bool colourFaces = true;
        //! A colour map for data plotting
        morph::ColourMap<float> cm;
        //! A scaling for data colour
        morph::Scale<T, float> colourScale;
        //! The number of iterations in the geodesic sphere. Set before finalize() to change from the default.
        int iterations = 2;
        //! This may be filled with the number of vertices in the geodesic
        int n_verts = 0;
        //! This may be filled with the number of faces in the geodesic
        int n_faces = 0;
    };

} // namespace morph
