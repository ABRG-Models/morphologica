/*
 * Test prog for geodesic polys
 */

#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/TriangleVisual.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

// function to subdivide triangles to make a geodesic
void subdivide_triangles (morph::vvec<morph::vec<float, 3>>& vertices,
                          morph::vvec<morph::vec<int, 3>>& faces, const int iterations = 1)
{
    // From iterations, we can compute the number of vertices, edges and faces
    // expected. (see https://en.wikipedia.org/wiki/Geodesic_polyhedron)
    const int T = std::pow (4, iterations);
    const int n_verts = 10 * T + 2;
    const int n_faces = 20 * T; // also, n_edges is 30T, but we don't need it

    // Special comparison function to order vertices in a Geodesic polyhedron
    auto _vtx_cmp = [](morph::vec<float, 3> a, morph::vec<float, 3> b)
    {
        // Compare first by vertex's z location
        bool is_less_than = false;
        if (std::abs(a[2] - b[2]) < 10 * std::numeric_limits<float>::epsilon()) {
            // and then by rotational angle in the x-y plane
            float angle_a = std::atan2 (a[1], a[0]);
            float angle_b = std::atan2 (b[1], b[0]);
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
    std::map<morph::vec<float, 3>, int, decltype(_vtx_cmp)> vertices_map(_vtx_cmp);
    std::map<int, int> idx_remap;

    static constexpr bool debug_vertices = false;
    static constexpr bool debug_faces = false;
    static constexpr bool debug_general = false;

    for (int i = 0; i < iterations; ++i) {

        // (Re)Populate vertices_map from vertices.
        int ii = 0;
        vertices_map.clear();
        for (auto v : vertices) { // original order of vertices (unordered)
            vertices_map[v] = ii++; // Orders into a new *good* order (spiral) and records the value of the original order.
        }

        if constexpr (debug_general) {
            std::cout << "ITERATION START\nOn iteration " << i << " vertices size is " << vertices.size() << std::endl;
        }
        std::map<morph::vec<float, 3>, // Comparing on the vertex key
                 morph::vec<int, 3>, decltype(_vtx_cmp)> faces_map(_vtx_cmp);
        int count = 0;
        int fcount = 0;
        if constexpr (debug_faces) {
            std::cout << "Looping through " << faces.size() << " faces...\n";
        }

        for (const auto f : faces) { // faces contains indexes into vertices.
            if constexpr (debug_faces) {
                std::cout << "Working on origin face " << fcount++ << " made of vertices " << f << std::endl;
                std::cout << "  which are: " << vertices[f[0]] << " / " << vertices[f[1]] << " / " << vertices[f[2]] << "\n";
            }
            morph::vec<float, 3> va = (vertices[f[1]] + vertices[f[0]]) / 2.0f;
            morph::vec<float, 3> vb = (vertices[f[2]] + vertices[f[1]]) / 2.0f;
            morph::vec<float, 3> vc = (vertices[f[0]] + vertices[f[2]]) / 2.0f;
            // Renormalize the new vertices, placing them on the surface of a sphere
            va.renormalize();
            vb.renormalize();
            vc.renormalize();

            // Is va/vb/vc new?

            int idx = vertices.size();
            int a = 0;
            try {
                a = vertices_map.at (va); // a is the existing index (old money)
                if constexpr (debug_vertices) {
                    std::cout << "a is EXISTING VERTEX " << va << ", index " << a << std::endl; // old money
                }

            } catch (const std::out_of_range& e) {
                a = idx++;
                vertices.push_back (va);
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
                b = idx++;
                vertices.push_back (vb);
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
                c = idx++;
                vertices.push_back (vc);
                vertices_map[vc] = c;
                if constexpr (debug_vertices) {
                    std::cout << "INSERTED NEW vertex " << vc << " into vertices_map with index " << c << "\n"; // old money
                }
            }

            // Now add to faces_map (keyed by centroid of each face) faces contains indices.
            auto add_face = [&faces_map, &count, &vertices] (const morph::vec<float, 3>& vA,
                                                             const morph::vec<float, 3>& vB,
                                                             const morph::vec<float, 3>& vC,
                                                             const morph::vec<int, 3>& newface,
                                                             const int newface_num)
            {
                morph::vec<float, 3> centroid = (vA + vB + vC) / 3.0f;
                if constexpr (debug_faces) {
                    if (faces_map.count (centroid) > 0u) {
                        std::stringstream ee;
                        ee << "The face " << newface
                           << ", vertices: " << vertices[newface[0]] << "," << vertices[newface[1]] << "," << vertices[newface[2]]
                           << ", centroid " << centroid << " already exists as a key in faces_map with value: " << faces_map[centroid]
                           << ", and vertices "
                           << vertices[faces_map[centroid][0]] << "," << vertices[faces_map[centroid][1]] << "," << vertices[faces_map[centroid][2]];
                        throw std::runtime_error (ee.str());
                    }
                }
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

            morph::vec<int, 3> newface = { f[0], a, c }; // indices in old money here
            add_face (vertices[f[0]], va, vc, newface, 1);

            newface = { f[1], b, a };
            add_face (vertices[f[1]], vb, va, newface, 2);

            newface = { f[2], c, b };
            add_face (vertices[f[2]], vc, vb, newface, 3);

            newface = { a, b, c };
            add_face (va, vb, vc, newface, 4);
        }

        if constexpr (debug_faces) {
            std::cout << "faces_map size after loop: " << faces_map.size() << std::endl;
        }

        // Copy faces_map back to faces?
        faces.resize (faces_map.size());
        int j = 0;
        for (auto fm : faces_map) {
            faces[j] = fm.second;
            j++;
        } // faces should now be correctly ordered
        if constexpr (debug_faces) { std::cout << "faces size " << faces.size() << std::endl; }

        // idx_remap is keyed on the badly ordered indices; value is correct ordering (j
        // follows order of vertices_map)
        int k = 0;
        idx_remap.clear();
        for (auto v : vertices_map) { idx_remap[v.second] = k++; }

        // faces is in the language of 'badly ordered indices'. We want it to be
        // expressed with the new ordered indices, inherent in the vertices_map
        // ordering, as we'll be writing data from vertices_map into vertices with the
        // correct ordering
        if constexpr (debug_faces) { std::cout << "before faces loop, faces size " << faces.size() << std::endl; }

        for (auto& _f : faces) {
            if constexpr (debug_faces) { std::cout << "Remapping face indices " << _f << " to "; }
            _f[0] = idx_remap[_f[0]]; // remap old money faces index to new money index
            _f[1] = idx_remap[_f[1]];
            _f[2] = idx_remap[_f[2]];
            if constexpr (debug_faces) {
                std::cout << _f << std::endl;
            }
        }
        if constexpr (debug_faces) { std::cout << "after faces loop, faces size " << faces.size() << std::endl; }

        // Populate vertices
        vertices.resize (vertices_map.size());
        int l = 0;
        for (auto v : vertices_map) {
            vertices[l++] = v.first;
        }
        if constexpr (debug_general) {
            std::cout << "At end, faces.size() = " << faces.size() << " and vertices.size() = " << vertices.size() << std::endl;
        }
    }

    // Check our structures against n_faces and n_verts
    std::cout << "vertices.size(): " << vertices.size() << " n_verts: " << n_verts << std::endl;
    if (static_cast<int>(vertices.size()) != n_verts) { throw std::runtime_error ("vertices has wrong size"); }
    std::cout << "faces.size(): " << faces.size() << " n_faces: " << n_faces << std::endl;
    if (static_cast<int>(faces.size()) != n_faces) { throw std::runtime_error ("faces has wrong size"); }
}

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Geodesic vertices");
    v.showCoordArrows = true;
    v.lightingEffects();

    // First create an icosahedron...
    morph::vvec<morph::vec<float, 3>> icoverts(12, {0.0f, 0.0f, 0.0f});
    morph::vvec<morph::vec<int, 3>> icofaces(20, {0, 0, 0});
    morph::geometry::icosahedron (icoverts, icofaces);
    // ...then make it into a geodesic polyhedron
    subdivide_triangles (icoverts, icofaces, 5);

    // Coordinates of face centres (for debug/viz)
    //morph::vvec<morph::vec<float, 3>> fcentres(icofaces.size(), {2.5f, 0.0f, 0.0f});
    morph::vvec<morph::vec<float, 3>> fcentres(icofaces.size(), {0.0f, 0.0f, 0.0f});
    for (unsigned int i = 0; i < icofaces.size(); ++i) {
        fcentres[i] += (icoverts[icofaces[i][0]] + icoverts[icofaces[i][1]] + icoverts[icofaces[i][2]])/3.0f;
    }

    try {
        morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        morph::Scale<float> scale;
        scale.setParams (1.0f, 0.0f);

        morph::vvec<float> data(icoverts.size(), 0.06f);
        morph::vvec<float> data2(icofaces.size(), 0.95f);
#if 0
        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&icoverts);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.005f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);
#endif
#if 0
        // Use a second scatter visual to show the centre of each face, numbered in a different colour
        auto sv2 = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv2);
        sv2->setDataCoords (&fcentres);
        sv2->setScalarData (&data2);
        sv2->radiusFixed = 0.006f;
        sv2->colourScale = scale;
        sv2->cm.setType (morph::ColourMapType::Plasma);
        sv2->labelIndices = true;
        sv2->labelOffset = { 0.01f, 0.0f, 0.0f };
        sv2->labelSize = 0.02f;
        sv2->finalize();
        v.addVisualModel (sv2);
#endif

#if 1
        // Triangle visuals for the faces
        morph::ColourMap<float> cm(morph::ColourMapType::Jet);
        for (unsigned int i = 0; i < icofaces.size(); ++i) {
            std::array<float, 3> colr = cm.convert (i/static_cast<float>(icofaces.size()));
            //std::cout << "Draw triangle with vertices: " << icofaces[i] << std::endl;
            auto tv = std::make_unique<morph::TriangleVisual<>> (offset,
                                                                 icoverts[icofaces[i][0]],
                                                                 icoverts[icofaces[i][1]],
                                                                 icoverts[icofaces[i][2]],
                                                                 colr);
            v.bindmodel (tv);
            tv->setAlpha (0.8f);
            tv->finalize();
            v.addVisualModel (tv);
        }
#endif
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
