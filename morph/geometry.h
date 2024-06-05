/*
 * Geometry algorithms. Initially, the code to generate icosahedrons and geodesic polyhedrons.
 */

#pragma once

#include <map>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <morph/mathconst.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Quaternion.h>
#include <morph/TransformMatrix.h>

namespace morph {
    namespace geometry {

        //! a container class for the vertices and faces of a polyhedron
        template<typename F>
        struct polyhedron
        {
            //! A list of the vertices
            morph::vvec<morph::vec<F, 3>> vertices;
            //! A list of the faces
            morph::vvec<morph::vec<int, 3>> faces;
        };

        //! Return a geometry::polyhedron object containing vertices and face indices for
        //! an icosahedron
        template<typename F>
        inline polyhedron<F> icosahedron()
        {
            constexpr F phi = (F{1} + std::sqrt(F{5})) / F{2};

            morph::geometry::polyhedron<F> ico;

            // Arranged 'in spiral order', going with positive angle in x/y plane (i.e. around z axis)
            ico.vertices = {
                { F{-1}, phi,   F{0}  },

                { F{1},  phi,   F{0}  },
                { F{0},  F{1},  -phi  },
                { -phi,  F{0},  F{-1} },
                { -phi,  F{0},  F{1}  },
                { F{0},  F{1},  phi   },

                { phi,   F{0},  F{1}  },
                { phi,   F{0},  F{-1} },
                { F{0},  F{-1}, -phi  },
                { F{-1}, -phi,  F{0}  },
                { F{0},  F{-1}, phi   },

                { F{1},  -phi,  F{0}  }
            };

            // Set up the transform matrix for our rotation, made up of a rotation about the z axis...
            morph::Quaternion<F> rotn1;
            rotn1.rotate (F{0}, F{0}, F{1}, std::atan2(F{1}, phi));
            // ...and a rotation about the x axis:
            morph::Quaternion<F> rotn2;
            rotn2.rotate (F{1}, F{0}, F{0}, -morph::mathconst<F>::pi_over_2);
            // We then translate the quaternions into a transform matrix:
            morph::TransformMatrix<F> rmat;
            rmat.rotate (rotn1 * rotn2);

            // For each vertex, apply rotational transform and renormalize
            morph::vec<F, 4> v4;
            for (auto& vertex : ico.vertices) {
                v4 = rmat * vertex;         // Apply the rotation (returns 4D vector)
                vertex = v4.less_one_dim(); // Extract 3D vector
                vertex.renormalize();       // Make it length 1
            }
            // after this, the vertex order is no longer spiral from top to bottom

            // Each face is defined by three vertices
            ico.faces = {
                {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 1},
                {5, 6, 1}, {1, 6, 7}, {1, 7, 2}, {2, 7, 8}, {2, 8, 3}, {3, 8, 9}, {3, 9, 4}, {4, 9, 10}, {4, 10, 5}, {5, 10, 6},
                {10, 11, 6}, {6, 11, 7}, {7, 11, 8}, {8, 11, 9}, {9, 11, 10}
            };

            return ico;
        }

        /*!
         * A function to subdivide the triangles of an icosahedron to make a geodesic
         * polynomial sphere. The inputs are the vertices (3D coordinates) and faces
         * (sets of 3 indices) that define an icosahedron. The inputs are resized and
         * re-populated with the vertices and faces of the geodesic sphere obtained by
         * sub-dividing the triangles of the icosahedron 'iterations' times. In
         * principle this algorithm could be applied to other input shapes, such as an
         * augmented dodecahedron ('augmentation' is the pre-processing step of
         * subdivision of the non-triangular faces into triangles). Hwoever, the runtime
         * tests for n_verts and n_faces assume the input shape was an icosahedron.
         *
         * Importantly, in this algorithm, the vertices and the faces are *ordered* in a
         * spiral order from the vertex coordinate (or face centroid coordinate) with
         * maximum z value down to the one with minimum z value. This generates memory
         * structures that are easier to iterate through and more useful for
         * visualization.
         *
         * \param geo (output). Pass in an empty polyhedron. This will be resized and
         * populated with teh vertices and faces of a geodesic polyhedron.
         *
         * \tparam F The floating point type for the coordinate elements. Use float or
         * double.
         *
         * \tparam iterations The number f times to subdivide the initial icosahedron
         */
        template<typename F>
        void icosahedral_geodesic (morph::geometry::polyhedron<F>& geo, const int iterations)
        {
            // From iterations, we can compute the number of vertices, edges and faces
            // expected. (see https://en.wikipedia.org/wiki/Geodesic_polyhedron)
            const int T = std::pow (4, iterations);
            const int n_verts = 10 * T + 2;
            const int n_faces = 20 * T; // also, n_edges is 30T, but we don't need it

            // Start out with an icosahedron
            if (geo.vertices.empty() && geo.faces.empty()) {
                geo = morph::geometry::icosahedron<F>();
            } else {
                throw std::runtime_error ("Pass in an empty polyhedron");
            }

            // A special comparison function to order vertices in our Geodesic polyhedron. The
            // vertices (or face centroids) are arranged in a spiral, from z_max to z_min,
            // spiralling anticlockwise in the x-y plane (that is, with decreasing value in the
            // z axis and with increasing angle in the x-y plane)
            auto _vtx_cmp = [](morph::vec<F, 3> a, morph::vec<F, 3> b)
            {
                constexpr F z_thresh = 10 * std::numeric_limits<F>::epsilon();
                // Compare first by vertex's z location
                bool is_less_than = false;
                if (std::abs(a[2] - b[2]) < z_thresh) {
                    // and then by rotational angle in the x-y plane
                    F angle_a = std::atan2 (a[1], a[0]);
                    F angle_b = std::atan2 (b[1], b[0]);
                    if (angle_a < angle_b) { is_less_than = true; }
                } else if (a[2] < b[2]) { // Put max z at start of array
                    is_less_than = false;
                } else {
                    is_less_than = true;
                }
                return is_less_than;
            };

            // Make a keyed container for the vertices, as we will need to reorder them. note:
            // morph::vec is key to map, as we will have a very custom sorting function. This
            // requires some care with the sorting function used by the std::map
            std::map<morph::vec<F, 3>, int, decltype(_vtx_cmp)> vertices_map(_vtx_cmp);
            std::map<int, int> idx_remap;

            constexpr bool debug_vertices = false;
            constexpr bool debug_faces = false;
            constexpr bool debug_general = false;

            for (int i = 0; i < iterations; ++i) {

                // (Re)Populate vertices_map from vertices.
                int ii = 0;
                vertices_map.clear();
                for (auto v : geo.vertices) { // original order of vertices (unordered)
                    vertices_map[v] = ii++; // Orders into a new *good* order (spiral) and records the value of the original order.
                }

                if constexpr (debug_general) {
                    std::cout << "ITERATION START\nOn iteration " << i << " vertices size is " << geo.vertices.size() << std::endl;
                }
                std::map<morph::vec<F, 3>, // Comparing on the vertex key
                         morph::vec<int, 3>, decltype(_vtx_cmp)> faces_map(_vtx_cmp);
                int count = 0;
                int fcount = 0;
                if constexpr (debug_faces) {
                    std::cout << "Looping through " << geo.faces.size() << " faces...\n";
                }

                // A function to add to faces_map (keyed by centroid of each face) faces contains indices.
                auto add_face = [&faces_map, &count] (const morph::vec<F, 3>& vA,
                                                      const morph::vec<F, 3>& vB,
                                                      const morph::vec<F, 3>& vC,
                                                      const morph::vec<int, 3>& newface,
                                                      const int newface_num)
                {
                    morph::vec<F, 3> centroid = (vA + vB + vC) / 3.0f;
                    faces_map[centroid] = newface;
                    count += 1;
                    if constexpr (debug_faces) {
                        std::cout << "\n" << newface_num << " Added a face [" << vA << " -- " << vB << " -- " << vC << "]\n"
                                  <<"        (" << ((vA + vB + vC) / 3.0f) << "). ";
                        std::cout << "\n" << newface_num << " Centroid: " << centroid;
                        std::cout << "\n" << newface_num << " Added a face ["
                                  << newface[0] << " -- " << newface[1] << " -- " << newface[2] << "]\n";
                        std::cout << "faces_map size should now be " << count << " but is: " << faces_map.size() << std::endl;
                    }
                    if (static_cast<size_t>(count) != faces_map.size()) { throw std::runtime_error ("count != faces_map.size()"); }
                };

                for (const auto f : geo.faces) { // faces contains indexes into vertices.
                    if constexpr (debug_faces) {
                        std::cout << "Working on origin face " << fcount++ << " made of vertices " << f << std::endl;
                        std::cout << "  which are: " << geo.vertices[f[0]] << " / " << geo.vertices[f[1]] << " / " << geo.vertices[f[2]] << "\n";
                    }
                    morph::vec<F, 3> va = (geo.vertices[f[1]] + geo.vertices[f[0]]) / 2.0f;
                    morph::vec<F, 3> vb = (geo.vertices[f[2]] + geo.vertices[f[1]]) / 2.0f;
                    morph::vec<F, 3> vc = (geo.vertices[f[0]] + geo.vertices[f[2]]) / 2.0f;
                    // Renormalize the new vertices, placing them on the surface of a sphere
                    va.renormalize();
                    vb.renormalize();
                    vc.renormalize();

                    // Is va/vb/vc new?

                    int a = 0;
                    try {
                        a = vertices_map.at (va); // a is the existing index (old money)
                        if constexpr (debug_vertices) {
                            std::cout << "a is EXISTING VERTEX " << va << ", index " << a << std::endl; // old money
                        }

                    } catch (const std::out_of_range& e) {
                        a = geo.vertices.size();
                        geo.vertices.push_back (va);
                        vertices_map[va] = a;
                        if constexpr (debug_vertices) {
                            std::cout << "INSERTED NEW vertex " << va << " into vertices_map with index " << a << "\n"; // old money
                        }
                    }

                    int b = 0;
                    try {
                        b = vertices_map.at (vb);
                        if constexpr (debug_vertices) {
                            std::cout << "b is EXISTING VERTEX " << vb << ", index " << b  << std::endl; // old money
                        }
                    } catch (const std::out_of_range& e) {
                        b = geo.vertices.size();
                        geo.vertices.push_back (vb);
                        vertices_map[vb] = b;
                        if constexpr (debug_vertices) {
                            std::cout << "INSERTED NEW vertex " << vb << " into vertices_map with index " << b << "\n"; // old money
                        }
                    }

                    int c = 0;
                    try {
                        c = vertices_map.at (vc);
                        if constexpr (debug_vertices) {
                            std::cout << "c is EXISTING VERTEX " << vc << ", index " << c << std::endl; // old money
                        }
                    } catch (const std::out_of_range& e) {
                        c = geo.vertices.size();
                        geo.vertices.push_back (vc);
                        vertices_map[vc] = c;
                        if constexpr (debug_vertices) {
                            std::cout << "INSERTED NEW vertex " << vc << " into vertices_map with index " << c << "\n"; // old money
                        }
                    }

                    morph::vec<int, 3> newface = { f[0], a, c }; // indices in old money here
                    add_face (geo.vertices[f[0]], va, vc, newface, 1);

                    newface = { f[1], b, a };
                    add_face (geo.vertices[f[1]], vb, va, newface, 2);

                    newface = { f[2], c, b };
                    add_face (geo.vertices[f[2]], vc, vb, newface, 3);

                    newface = { a, b, c };
                    add_face (va, vb, vc, newface, 4);
                }

                if constexpr (debug_faces) {
                    std::cout << "faces_map size after loop: " << faces_map.size() << std::endl;
                }

                // Copy faces_map back to faces?
                geo.faces.resize (faces_map.size());
                int j = 0;
                for (auto fm : faces_map) {
                    geo.faces[j] = fm.second;
                    j++;
                } // faces should now be correctly ordered
                if constexpr (debug_faces) { std::cout << "faces size " << geo.faces.size() << std::endl; }

                // idx_remap is keyed on the badly ordered indices; value is correct ordering (j
                // follows order of vertices_map)
                int k = 0;
                idx_remap.clear();
                for (auto v : vertices_map) { idx_remap[v.second] = k++; }

                // faces is in the language of 'badly ordered indices'. We want it to be
                // expressed with the new ordered indices, inherent in the vertices_map
                // ordering, as we'll be writing data from vertices_map into vertices with the
                // correct ordering
                if constexpr (debug_faces) { std::cout << "before faces loop, faces size " << geo.faces.size() << std::endl; }

                for (auto& _f : geo.faces) {
                    if constexpr (debug_faces) { std::cout << "Remapping face indices " << _f << " to "; }
                    _f[0] = idx_remap[_f[0]]; // remap old money faces index to new money index
                    _f[1] = idx_remap[_f[1]];
                    _f[2] = idx_remap[_f[2]];
                    if constexpr (debug_faces) { std::cout << _f << std::endl; }
                }
                if constexpr (debug_faces) { std::cout << "after faces loop, faces size " << geo.faces.size() << std::endl; }

                // Populate vertices
                geo.vertices.resize (vertices_map.size());
                int l = 0;
                for (auto v : vertices_map) {
                    geo.vertices[l++] = v.first;
                }
                if constexpr (debug_general) {
                    std::cout << "At end, faces.size() = " << geo.faces.size() << " and vertices.size() = " << geo.vertices.size() << std::endl;
                }
            }

            // Check our structures against n_faces and n_verts
            if constexpr (debug_general) {
                std::cout << "vertices.size(): " << geo.vertices.size() << " n_verts: " << n_verts << std::endl;
            }
            if (static_cast<int>(geo.vertices.size()) != n_verts) { throw std::runtime_error ("vertices has wrong size"); }
            if constexpr (debug_general) {
                std::cout << "faces.size(): " << geo.faces.size() << " n_faces: " << n_faces << std::endl;
            }
            if (static_cast<int>(geo.faces.size()) != n_faces) { throw std::runtime_error ("faces has wrong size"); }
        }

    } // geometry
} // morph
