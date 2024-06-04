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
                          morph::vvec<morph::vec<int, 3>>& faces, const int iterations = 1, const int itstage = 1)
{
    // From iterations, we can compute the number of vertices, edges and faces
    // expected. (see https://en.wikipedia.org/wiki/Geodesic_polyhedron)
    const int T = std::pow (4, itstage);
    const int n_verts = 10 * T + 2;
    const int n_faces = 20 * T; // also, n_edges is 30T, but we don't need it

    // Get a characteristic length scale from vertices for the icosahedron
    const float length_scale = (vertices[faces[0][0]] - vertices[faces[0][1]]).length();
    const float z_thresh = (length_scale / std::sqrt(T)) * 0.1f;

    // Special comparison function to order vertices in a Geodesic polyhedron
    auto _vtx_cmp = [z_thresh](morph::vec<float, 3> a, morph::vec<float, 3> b)
    {
        //std::cout << "_vtx_cmp (" <<a << "<" << b << "?)\n";
        // Compare first by vertex's z location
        bool is_less_than = false;
        if (std::abs(a[2] - b[2]) < z_thresh) {
            // and then by rotational angle in the x-y plane
            float angle_a = std::atan2 (a[1], a[0]);
            float angle_b = std::atan2 (b[1], b[0]);
            //std::cout << "angle_a: " << angle_a << " and angle_b: " << angle_b << " a[2]: " << a[2] << ", b[2]: " << b[2];
            if (angle_a < angle_b) {
                //std::cout << "a is < b on account of angle\n";
                is_less_than = true;
            }
        } else if (a[2] < b[2]) { // Put max z at start of array
            //std::cout << "a is NOT < b on account of z component";
            is_less_than = false;
        } else {
            //std::cout << "a is < b on account of z component";
            is_less_than = true;
        }
        //std::cout << " | a < b : " << (is_less_than ? "T" : "F") << std::endl;
        return is_less_than;
    };

    // Make a keyed container for the vertices, as we will need to reorder them. note:
    // morph::vec is key to map, as we will have a very custom sorting function. This
    // requires some care with the sorting function used by the std::map
    std::map<morph::vec<float, 3>, int, decltype(_vtx_cmp)> vertices_map(_vtx_cmp);

    std::map<int, int> idx_remap;

    // First, copy the initial vertices (from the icosahedron) into the vertices_map
    int i = 0;
    std::cout << "Copy vertices into vertices_map.\n";
    for (auto v : vertices) { // original order of vertices (unordered)
        vertices_map[v] = i++; // Orders into a new *good* order (spiral) and records the value of the original order.
    }
    std::cout << "Icosahedron vertex order:\n";
    int kkk = 0;
    for (auto v : vertices_map) { // Outputs in spiral order of vertices_map
        std::cout << kkk++ << ": vertices_map[" << v.first << "] = " << v.second <<  std::endl;
    }
    static constexpr bool debug_vertices = false;
    static constexpr bool debug_faces = false;

    for (i = 0; i < iterations; ++i) {
        std::cout << "ITERATION START\nOn iteration " << i << " vertices size is " << vertices.size() << std::endl;
        std::map<morph::vec<float, 3>, // Comparing on the vertex key
                 morph::vec<int, 3>, decltype(_vtx_cmp)> faces_map(_vtx_cmp);
        int count = 0;
        int fcount = 0;
        if constexpr (debug_faces) {
            std::cout << "Looping through " << faces.size() << " faces...\n";
        }
        std::cout << "Before looping faces, vertex order:\n";
        int kk = 0;
        for (auto v : vertices_map) { // Outputs in spiral order of vertices_map
            std::cout << kk++ << ": vertices_map[" << v.first << "] = " << v.second <<  std::endl;
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
            int a = 0;
            try {
                a = vertices_map.at (va); // a is the existing index (old money)
                if constexpr (debug_vertices) {
                    std::cout << "a is EXISTING VERTEX " << va << ", index " << a << std::endl; // old money
                }
            } catch (const std::out_of_range& e) {

                a = vertices.size();
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
                b = vertices.size();
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
                    std::cout << "c is EXISTING VERTEX " << vc  << ", index " << c << std::endl; // old money
                }
            } catch (const std::out_of_range& e) {
                c = vertices.size();
                vertices.push_back (vc);
                vertices_map[vc] = c;
                if constexpr (debug_vertices) {
                    std::cout << "INSERTED NEW vertex " << vc << " into vertices_map with index " << c << "\n"; // old money
                }
            }

            // Now add to faces_map (keyed by centroid of each face) faces contains indices.
            faces_map[(vertices[f[0]] + va + vc) / 3.0f] = { f[0], a, c }; // indices in old money here
            count += 1;
            if constexpr (debug_faces) {
                std::cout << "\n1 Added a face [" << vertices[f[0]] << " -- " << va << " -- " << vc << "]\n        (" << ((vertices[f[0]] + va + vc) / 3.0f) << "). ";
                std::cout << "\n1 Added a face [" << f[0] << " -- " << a << " -- " << c << "]\n        (" << ((vertices[f[0]] + va + vc) / 3.0f) << "). ";
                std::cout << "faces_map size should now be " << count << " but is: " << faces_map.size() << std::endl;
            }
            if (static_cast<size_t>(count) != faces_map.size()) { throw std::runtime_error ("count != faces_map.size()"); }

            faces_map[(vertices[f[1]] + vb + va) / 3.0f] = { f[1], b, a };
            count += 1;
            if constexpr (debug_faces) {
                //std::cout << "\n2 Added a face [" << vertices[f[1]] << " -- " << vb << " -- " << va << "]\n        (" << ((vertices[f[1]] + vb + va) / 3.0f) << "). ";
                std::cout << "\n2 Added a face [" << f[1] << " -- " << b << " -- " << a << "]\n        (" << ((vertices[f[1]] + vb + va) / 3.0f) << "). ";
                std::cout << "faces_map size should now be " << count << " but is: " << faces_map.size() << std::endl;
            }
            if (static_cast<size_t>(count) != faces_map.size()) { throw std::runtime_error ("count != faces_map.size()"); }

            faces_map[(vertices[f[2]] + vc + vb) / 3.0f] = { f[2], c, b };
            count += 1;
            if constexpr (debug_faces) {
                //std::cout << "\n3 Added a face [" << vertices[f[2]] << " -- " << vc << " -- " << vb << "]\n        (" << ((vertices[f[2]] + vc + vb) / 3.0f) << "). ";
                std::cout << "\n3 Added a face [" << f[2] << " -- " << c << " -- " << b << "]\n        (" << ((vertices[f[2]] + vc + vb) / 3.0f) << "). ";
                std::cout << "faces_map size should now be " << count << " but is: " << faces_map.size() << std::endl;
            }
            if (static_cast<size_t>(count) != faces_map.size()) { throw std::runtime_error ("count != faces_map.size()"); }

            faces_map[(    va         + vb + vc) / 3.0f] = {  a  , b, c };
            count += 1;
            if constexpr (debug_faces) {
                std::cout << "\n4 Added a face [" << va << " -- " << vb << " -- " << vc << "]\n        (" << ((va + vb + vc) / 3.0f) << "). ";
                std::cout << "\n4 Added a face [" << a << " -- " << b << " -- " << c << "]\n        (" << ((va + vb + vc) / 3.0f) << "). ";
                std::cout << "faces_map size should now be " << count << " but is: " << faces_map.size() << std::endl;
            }
            if (static_cast<size_t>(count) != faces_map.size()) { throw std::runtime_error ("count != faces_map.size()"); }
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
        if constexpr (debug_faces) {
            std::cout << "faces size " << faces.size() << std::endl;
        }

#define REINDEX 1
#ifdef REINDEX
        // idx_remap is keyed on the badly ordered indices; value is correct ordering (j
        // follows order of vertices_map)
        int k = 0;
        idx_remap.clear();
        std::cout << "Create idx_remap...\n";
        for (auto v : vertices_map) {
            std::cout << "set idx_remap[v.second=" << v.second << "] = " << k << " for v.first = " << v.first <<  std::endl;
            idx_remap[v.second] = k++;
        }
        std::cout << "idx_remap.size(): " << idx_remap.size() << std::endl;

        // faces is in the language of 'badly ordered indices'. We want it to be
        // expressed with the new ordered indices, inherent in the vertices_map
        // ordering, as we'll be writing data from vertices_map into vertices with the
        // correct ordering
        if constexpr (debug_faces) { std::cout << "before faces loop, faces size " << faces.size() << std::endl; }

        for (auto& _f : faces) {
            if constexpr (debug_faces) {
                std::cout << "Remapping face indices " << _f << " to ";
            }
            std::cout << "f0 vertex index " << _f[0] << " in the old vertices becomes " << idx_remap[_f[0]] << " in the new\n";
            _f[0] = idx_remap[_f[0]]; // remap old money faces index to new money index
            std::cout << "f1 vertex index " << _f[1] << " in the old vertices becomes " << idx_remap[_f[1]] << " in the new\n";
            _f[1] = idx_remap[_f[1]];
            std::cout << "f2 vertex index " << _f[2] << " in the old vertices becomes " << idx_remap[_f[2]] << " in the new\n";
            _f[2] = idx_remap[_f[2]];
            if constexpr (debug_faces) {
                std::cout << _f << std::endl;
            }
        }
        if constexpr (debug_faces) {
            std::cout << "after faces loop, faces size " << faces.size() << std::endl;
        }

        // Populate vertices
        vertices.resize (vertices_map.size());
        int l = 0;
        std::cout << "Re-creating vertices container...\n";
        for (auto v : vertices_map) {
            std::cout << "vertices["<<l<<"] gets the next key from vertex_map: " << v.first << " old order: " << v.second << std::endl;
            vertices[l++] = v.first;
        }
#else
        vertices.resize (vertices_map.size());
        for (auto v : vertices_map) { vertices[v.second] = v.first; }
#endif
        std::cout << "At end of loop, faces.size() = " << faces.size()
                  << " and vertices.size() = " << vertices.size() << std::endl;
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
    subdivide_triangles (icoverts, icofaces, 1, 1);
    subdivide_triangles (icoverts, icofaces, 1, 2);
    subdivide_triangles (icoverts, icofaces, 1, 3);

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
        sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&fcentres);
        sv->setScalarData (&data2);
        sv->radiusFixed = 0.006f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);
#endif

#if 1
        // Triangle visuals for the faces
        morph::ColourMap<float> cm(morph::ColourMapType::Jet);
        for (unsigned int i = 0; i < icofaces.size(); ++i) {
            std::array<float, 3> colr = cm.convert (i/static_cast<float>(icofaces.size()));
            std::cout << "Draw triangle with vertices: " << icofaces[i] << std::endl;
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
