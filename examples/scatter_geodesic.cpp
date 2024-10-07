/*
 * Test prog for geodesic polys
 */

#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/QuiverVisual.h>
#include <morph/TriangleVisual.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/geometry.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <tuple>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Geodesic vertices");
    v.showCoordArrows = true;
    v.lightingEffects();

    constexpr int iterations = 3;

    // First create an empty polyhedron object
    // ...then pass it into a geodesic polyhedron creation function
    morph::geometry::icosahedral_geodesic<float> geo = morph::geometry::make_icosahedral_geodesic<float> (iterations);

    // Coordinates of face centres (for debug/viz)
    morph::vvec<morph::vec<float, 3>> fcentres = geo.poly.get_face_centres();

    // Compute neighbour vectors from neighbour indices
    morph::vvec<morph::vvec<std::tuple<morph::vec<float, 3>, int>>> vneighb_vertices_with_indices = geo.get_neighbour_hexdir_vectors();

    // The tuples can't be passed directly to QuiverVisual
    morph::vvec<morph::vvec<morph::vec<float, 3>>> vneighb_vertices (vneighb_vertices_with_indices.size());
    for (unsigned int i = 0; i < vneighb_vertices_with_indices.size(); ++i) {
        vneighb_vertices[i].resize (3);
        // Debug/verify the neighbour *vertex indices* with these std::couts:
        //std::cout << "Vertex " << i << " has RGB neighbour indices: ";
        for (unsigned int j = 0; j < 3; ++j) {
            //std::cout << std::get<int>(vneighb_vertices_with_indices[i][j]) << ",";
            vneighb_vertices[i][j] = std::get<morph::vec<float, 3>>(vneighb_vertices_with_indices[i][j]);
        }
        //std::cout << std::endl;
    }

    // Control what's shown
    constexpr bool show_vertices = true;
    constexpr bool show_vertex_labels = true;
    constexpr bool show_face_centres = false;
    constexpr float face_alpha = 0.95f; // How much see-throughness? 1 for none.
    constexpr bool show_face_centre_labels = false;
    constexpr bool show_faces = true;
    constexpr bool show_neighbour_vectors = true;

    try {
        morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        morph::Scale<float> scale;
        scale.setParams (1.0f, 0.0f);

        morph::vvec<float> data(geo.poly.vertices.size(), 0.06f);
        morph::vvec<float> data2(geo.poly.faces.size(), 0.95f);

        if constexpr (show_vertices == true) {
            auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
            v.bindmodel (sv);
            sv->setDataCoords (&geo.poly.vertices);
            sv->setScalarData (&data);
            sv->radiusFixed = 0.005f;
            sv->colourScale = scale;
            sv->cm.setType (morph::ColourMapType::Plasma);
            sv->labelIndices = show_vertex_labels;
            sv->labelOffset = { 0.015f, 0.0f, 0.0f };
            sv->finalize();
            v.addVisualModel (sv);
        }

        if constexpr (show_face_centres == true) {
            // Use a second scatter visual to show the centre of each face, numbered in a different colour
            auto sv2 = std::make_unique<morph::ScatterVisual<float>> (offset);
            v.bindmodel (sv2);
            sv2->setDataCoords (&fcentres);
            sv2->setScalarData (&data2);
            sv2->radiusFixed = 0.006f;
            sv2->colourScale = scale;
            sv2->cm.setType (morph::ColourMapType::Plasma);
            sv2->labelIndices = show_face_centre_labels;
            sv2->labelOffset = { 0.01f, 0.0f, 0.0f };
            sv2->labelSize = 0.02f;
            sv2->finalize();
            v.addVisualModel (sv2);
        }

        if constexpr (show_faces == true) {
            // Triangle visuals for the faces
            morph::ColourMap<float> cm(morph::ColourMapType::Greyscale);
            for (unsigned int i = 0; i < geo.poly.faces.size(); ++i) {
                std::array<float, 3> colr = cm.convert (i/static_cast<float>(geo.poly.faces.size()));
                //std::cout << "Draw triangle with vertices: " << geo.faces[i] << std::endl;
                auto tv = std::make_unique<morph::TriangleVisual<>> (offset,
                                                                     geo.poly.vertices[geo.poly.faces[i][0]],
                                                                     geo.poly.vertices[geo.poly.faces[i][1]],
                                                                     geo.poly.vertices[geo.poly.faces[i][2]],
                                                                     colr);
                v.bindmodel (tv);
                tv->setAlpha (face_alpha);
                tv->finalize();
                v.addVisualModel (tv);
            }
        }

        if constexpr (show_neighbour_vectors == true) {
            // For each vertex, one QuiverVisual for the immediate neighbour vertices.
            morph::vvec<float> clrs;
            for (unsigned int i = 0; i < vneighb_vertices.size(); ++i) {
                morph::vvec<morph::vec<float, 3>> coords (vneighb_vertices[i].size(), geo.poly.vertices[i]);
                auto vmp = std::make_unique<morph::QuiverVisual<float>>(&coords, offset, &vneighb_vertices[i],
                                                                        morph::ColourMapType::Rainbow);
                v.bindmodel (vmp);
                clrs.linspace (0, 0.66, vneighb_vertices[i].size());
                vmp->scalarData = &clrs;
                vmp->colourScale.compute_scaling (0, 1);
                vmp->do_quiver_length_scaling = false; // Don't (auto)scale the lengths of the vectors
                vmp->quiver_length_gain = 0.5f;        // Apply a fixed gain to the length of the quivers on screen
                vmp->fixed_quiver_thickness = 0.01f/iterations;  // Fixed quiver thickness
                vmp->finalize();
                v.addVisualModel (vmp);
            }
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
