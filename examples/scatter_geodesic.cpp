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
#include <morph/geometry.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Geodesic vertices");
    v.showCoordArrows = true;
    v.lightingEffects();

    // First create an empty polyhedron object
    morph::geometry::polyhedron<float> geo;
    // ...then pass it into a geodesic polyhedron creation function
    morph::geometry::icosahedral_geodesic<float, 3> (geo);

    // Coordinates of face centres (for debug/viz)
    //morph::vvec<morph::vec<float, 3>> fcentres(icofaces.size(), {2.5f, 0.0f, 0.0f});
    morph::vvec<morph::vec<float, 3>> fcentres(geo.faces.size(), {0.0f, 0.0f, 0.0f});
    for (unsigned int i = 0; i < geo.faces.size(); ++i) {
        fcentres[i] += (geo.vertices[geo.faces[i][0]] + geo.vertices[geo.faces[i][1]] + geo.vertices[geo.faces[i][2]])/3.0f;
    }

    try {
        morph::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        morph::Scale<float> scale;
        scale.setParams (1.0f, 0.0f);

        morph::vvec<float> data(geo.vertices.size(), 0.06f);
        morph::vvec<float> data2(geo.faces.size(), 0.95f);
#if 1
        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&geo.vertices);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.005f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->labelOffset = { 0.015f, 0.0f, 0.0f };
        sv->finalize();
        v.addVisualModel (sv);
#endif
#if 1
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
        for (unsigned int i = 0; i < geo.faces.size(); ++i) {
            std::array<float, 3> colr = cm.convert (i/static_cast<float>(geo.faces.size()));
            //std::cout << "Draw triangle with vertices: " << geo.faces[i] << std::endl;
            auto tv = std::make_unique<morph::TriangleVisual<>> (offset,
                                                                 geo.vertices[geo.faces[i][0]],
                                                                 geo.vertices[geo.faces[i][1]],
                                                                 geo.vertices[geo.faces[i][2]],
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
