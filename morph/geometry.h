/*
 * Geometry algorithms. Initially, the code to generate icosahedrons and geodesic polyhedrons.
 */

#pragma once

#include <map>
#include <set>
#include <algorithm>
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

        /*!
         * a container class for the vertices and faces of a polyhedron
         */
        template<typename F>
        struct polyhedron
        {
            //! A list of the vertices
            morph::vvec<morph::vec<F, 3>> vertices;
            //! A list of the faces
            morph::vvec<morph::vec<int, 3>> faces;
        };

        /*!
         * A return information struct for the icosahedral_geodesic function. The
         * fivefold_vertices information *could* be part of a derived class icosahedron
         * (deriving from polyhedron). Polyhedron already contains n_vertices and
         * n_faces (as the size()s of vertices and faces)
         */
        struct geodesic_info
        {
            int n_vertices = 0;
            int n_faces = 0;
            std::set<int> fivefold_vertices; // Should end up with size 12
        };

        /*!
         * Return a geometry::polyhedron object containing vertices and face indices for
         * an icosahedron
         */
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
         * A function to get the number of faces and vertices that we'll generate when
         * we call icosahedral_geodesic(..., iterations)
         *
         * \param iterations The number of times the initial icosahedron will be subdivided
         */
        morph::geometry::geodesic_info
        icosahedral_geodesic_numbers (const int iterations)
        {
            morph::geometry::geodesic_info gi;
            // From iterations, we can compute the number of vertices, edges and faces
            // expected. (see https://en.wikipedia.org/wiki/Geodesic_polyhedron)
            const int T = std::pow (4, iterations);
            gi.n_vertices = 10 * T + 2;
            gi.n_faces = 20 * T;
            return gi;
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
         * \param iterations The number of times to subdivide the initial icosahedron
         *
         * \return A struct containing info about the geodesic. n_faces, n_verts and
         * indices of the five-fold symmetry vertices.
         */
        template<typename F>
        morph::geometry::geodesic_info
        icosahedral_geodesic (morph::geometry::polyhedron<F>& geo, const int iterations)
        {
            morph::geometry::geodesic_info gi = morph::geometry::icosahedral_geodesic_numbers (iterations);

            // Start out with an icosahedron
            if (geo.vertices.empty() && geo.faces.empty()) {
                geo = morph::geometry::icosahedron<F>();
            } else {
                throw std::runtime_error ("Pass in an empty polyhedron");
            }
            // As geo is currently an icosahedron, all vertices in geo are five-fold at this point
            for (int v = 0; v < static_cast<int>(geo.vertices.size()); ++v) { gi.fivefold_vertices.insert (v); }

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
                    if (i == 0) {
                        // On 0th iteration, all vertices are five-fold symmetric.
                        gi.fivefold_vertices.insert (ii);
                    }
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
                std::set<int> ffv; // temporary storage for a new fivefold_vertices set
                for (auto v : vertices_map) {
                    // See if we are remapping a fivefold vertex
                    if (gi.fivefold_vertices.count (v.second)) {
                        // Then v.second is a fivefold vertex, so insert its replacement, k, into ffv
                        // std::cout << "Replace old fivefold vertex index " << v.second << " with new one " << k << std::endl;
                        ffv.insert (k);
                    }
                    idx_remap[v.second] = k++;
                }
                // swap new five fold vertices into gi object
                std::swap (ffv, gi.fivefold_vertices);

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
                std::cout << "vertices.size(): " << geo.vertices.size() << " n_verts: " << gi.n_vertices << std::endl;
            }
            if (static_cast<int>(geo.vertices.size()) != gi.n_vertices) { throw std::runtime_error ("vertices has wrong size"); }
            if constexpr (debug_general) {
                std::cout << "faces.size(): " << geo.faces.size() << " n_faces: " << gi.n_faces << std::endl;
            }
            if (static_cast<int>(geo.faces.size()) != gi.n_faces) { throw std::runtime_error ("faces has wrong size"); }

            if constexpr (debug_general) {
                std::cout << "At end, gi.fivefold_vertices:\n";
                for (auto ffv : gi.fivefold_vertices) {
                    std::cout << "    vertex " << ffv << "\n";
                }
            }
            return gi;
        }

    } // geometry

    //! Experimental constexpr-capable geometry
    namespace geometry_ce {

        //! a container class for the vertices and faces of a polyhedron. Note this contains vec not vvec
        template<typename F, int n_verts, int n_faces>
        struct polyhedron
        {
            //! A list of the vertices
            morph::vec<morph::vec<F, 3>, n_verts> vertices;
            //! A list of the faces
            morph::vec<morph::vec<int, 3>, n_faces> faces;
        };

        // Container class specific to icosahedral geodesic
        template<typename F, int iterations>
        struct icosahedral_geodesic
        {
            static constexpr int T = std::pow(4, iterations);
            static constexpr int n_verts = 10 * T + 2;
            static constexpr int n_faces = 20 * T;
            geometry_ce::polyhedron <F, n_verts, n_faces> poly;
            morph::vec<int, 12> fivefold_vertices;
        };

        //! Return a geometry::polyhedron object containing vertices and face indices
        //! for an icosahedron. To compute the vertices, I ran the non-constexpr capable
        //! version in morph::geometry and transcribed the vertex numbers - a bit tedious.
        template<typename F>
        constexpr polyhedron<F, 12, 20> icosahedron()
        {
            morph::geometry_ce::polyhedron<F, 12, 20> ico = {
                { // vertices that have been rotated (they're not the same as the starters in geometry::icosahedron)
                    morph::vec<F, 3>({ F{0},                    F{0},                    F{1}                    }),
                    morph::vec<F, 3>({ F{0.89442719099991597},  F{0},                    F{0.44721359549995798}  }),
                    morph::vec<F, 3>({ F{0.27639320225002101},  F{0.85065080835203999},  F{0.44721359549995771}  }),
                    morph::vec<F, 3>({ F{-0.72360679774997894}, F{0.5257311121191337},   F{0.44721359549995782}  }),
                    morph::vec<F, 3>({ F{-0.72360679774997894}, F{-0.52573111211913348}, F{0.44721359549995809}  }),
                    morph::vec<F, 3>({ F{0.27639320225002101},  F{-0.85065080835203999}, F{0.44721359549995809}  }),
                    morph::vec<F, 3>({ F{0.72360679774997894},  F{-0.5257311121191337},  F{-0.44721359549995782} }),
                    morph::vec<F, 3>({ F{0.72360679774997894},  F{0.52573111211913348},  F{-0.44721359549995809} }),
                    morph::vec<F, 3>({ F{-0.27639320225002101}, F{0.85065080835203999},  F{-0.44721359549995809} }),
                    morph::vec<F, 3>({ F{-0.89442719099991597}, F{0},                    F{-0.44721359549995798} }),
                    morph::vec<F, 3>({ F{-0.27639320225002101}, F{-0.85065080835203999}, F{-0.44721359549995771} }),
                    morph::vec<F, 3>({ F{0},                    F{0},                    F{-1}                   })
                },
                { // Each face is defined by three vertices
                    morph::vec<int, 3>({0, 1, 2}),
                    morph::vec<int, 3>({0, 2, 3}),
                    morph::vec<int, 3>({0, 3, 4}),
                    morph::vec<int, 3>({0, 4, 5}),
                    morph::vec<int, 3>({0, 5, 1}),

                    morph::vec<int, 3>({5, 6, 1}),
                    morph::vec<int, 3>({1, 6, 7}),
                    morph::vec<int, 3>({1, 7, 2}),
                    morph::vec<int, 3>({2, 7, 8}),
                    morph::vec<int, 3>({2, 8, 3}),
                    morph::vec<int, 3>({3, 8, 9}),
                    morph::vec<int, 3>({3, 9, 4}),
                    morph::vec<int, 3>({4, 9, 10}),
                    morph::vec<int, 3>({4, 10, 5}),
                    morph::vec<int, 3>({5, 10, 6}),

                    morph::vec<int, 3>({10, 11, 6}),
                    morph::vec<int, 3>({6, 11, 7}),
                    morph::vec<int, 3>({7, 11, 8}),
                    morph::vec<int, 3>({8, 11, 9}),
                    morph::vec<int, 3>({9, 11, 10})
                }
            };

            return ico;
        }

        //! Experimental for constexpr. No ordering of vertices as in morph::geometry version. DO want to test for duplicate vertices.
        template<typename F, int iterations>
        constexpr morph::geometry_ce::icosahedral_geodesic<F, iterations> make_icosahedral_geodesic()
        {
            morph::geometry_ce::icosahedral_geodesic<F, iterations> geo; // our return object

            // geo must be fully initialized in a constexpr function (this could be constructor in icosahedral_geodesic<>
            constexpr morph::vec<int, 3> fz = { 0, 0, 0 };
            constexpr morph::vec<F, 3> vz = { F{0}, F{0}, F{0} };
            for (int i = 0; i < geo.poly.faces.size(); ++i) { geo.poly.faces[i] = fz; }
            for (int i = 0; i < geo.poly.vertices.size(); ++i) { geo.poly.vertices[i] = vz; }

            // Start out with an icosahedron
            constexpr morph::geometry_ce::polyhedron<F, 12, 20> initial_ico = morph::geometry_ce::icosahedron<F>();

            // copy initial_ico into geo.poly As geo is currently an icosahedron, all
            // vertices in geo are five-fold at this point, so populate this (and then
            // leave it alone)
            for (int i = 0; i < 12; ++i) {
                geo.poly.vertices[i] = initial_ico.vertices[i];
                geo.fivefold_vertices[i] = i;
            }
            for (int i = 0; i < 20; ++i) {
                geo.poly.faces[i] = initial_ico.faces[i];
            }

            for (int i = 0; i < iterations; ++i) {

                // Compute n_verts/n_faces for current iterations i
                int _T = std::pow(4, i);
                int _n_verts = 10 * _T + 2; // i=0; 12 i=1; 42
                int _n_faces = 20 * _T;     // i=0; 20 i=1; 80

                // This will need to be geo.faces for the right *stage* of the process
                // OR iterate with an i that is the right size
                //for (const auto f : geo.faces) { // faces contains indexes into vertices.
                for (int f = 0; f < _n_faces; ++f) {
                    //std::cout << "f: " << f << " _n_faces: " << _n_faces << " geo.poly.faces.size(): " << geo.poly.faces.size() << std::endl;
                    morph::vec<F, 3> va = (geo.poly.vertices[geo.poly.faces[f][1]] + geo.poly.vertices[geo.poly.faces[f][0]]) / 2.0f;
                    morph::vec<F, 3> vb = (geo.poly.vertices[geo.poly.faces[f][2]] + geo.poly.vertices[geo.poly.faces[f][1]]) / 2.0f;
                    morph::vec<F, 3> vc = (geo.poly.vertices[geo.poly.faces[f][0]] + geo.poly.vertices[geo.poly.faces[f][2]]) / 2.0f;
                    va.renormalize(); // constexpr capable function in morph::vec
                    vb.renormalize();
                    vc.renormalize();

                    // Is va/vb/vc new?
                    int a = -1;
                    for (int v = 0; v < _n_verts; ++v) {
                        if (geo.poly.vertices[v][0] == va[0] && geo.poly.vertices[v][1] == va[1] && geo.poly.vertices[v][2] == va[2]) {
                            // va is not a new vertex, set a to be the correct index
                            a = v;
                        }
                    }
                    if (a == -1) {
                        // va is a new vertex, add ono to poly.vertices
                        geo.poly.vertices[_n_verts] = va;
                        a = _n_verts;
                        _n_verts++;
                    }

                    int b = -1;
                    for (int v = 0; v < _n_verts; ++v) {
                        if (geo.poly.vertices[v][0] == vb[0] && geo.poly.vertices[v][1] == va[1] && geo.poly.vertices[v][2] == va[2]) {
                            // va is not a new vertex, set a to be the correct index
                            b = v;
                        }
                    }
                    if (b == -1) {
                        // va is a new vertex, add ono to poly.vertices
                        geo.poly.vertices[_n_verts] = vb;
                        b = _n_verts;
                        _n_verts++;
                    }

                    int c = -1;
                    for (int v = 0; v < _n_verts; ++v) {
                        if (geo.poly.vertices[v][0] == vc[0] && geo.poly.vertices[v][1] == va[1] && geo.poly.vertices[v][2] == va[2]) {
                            // va is not a new vertex, set a to be the correct index
                            c = v;
                        }
                    }
                    if (c == -1) {
                        // va is a new vertex, add ono to poly.vertices
                        geo.poly.vertices[_n_verts] = vc;
                        c = _n_verts;
                        _n_verts++;
                    }

                    morph::vec<int, 3> newface = { geo.poly.faces[f][0], a, c };
                    //std::cout << "add " << newface << " to poly.faces (size " << geo.poly.faces.size() << ") at index " << _n_faces << std::endl;
                    int next_face = _n_faces;
                    geo.poly.faces[next_face++] = newface;
                    newface = { geo.poly.faces[f][1], b, a };
                    geo.poly.faces[next_face++] = newface;
                    newface = { geo.poly.faces[f][2], c, b };
                    geo.poly.faces[next_face++] = newface;
                    newface = { a, b, c };
                    geo.poly.faces[next_face++] = newface;
                }
            }

            return geo;
        }
    }

} // morph
