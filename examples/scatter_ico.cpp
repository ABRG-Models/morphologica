/*
 * This program was a test prog to help me decide how to make
 * VisualModel::computeIcosahedron. It draws a scatter plot of icosahedron vertices,
 * triangles for the faces and another set of scatter plot spheres for the face centres.
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

    morph::Visual v(1024, 768, "morph::ScatterVisual");
    v.showCoordArrows = true;
    //v.coordArrowsInScene = true;
    v.lightingEffects();

    morph::geometry::polygon<float> ico = morph::geometry::icosahedron<float>();

    // Coordinates of face centres
    morph::vvec<morph::vec<float, 3>> fcentres(20, {0.0, 0.0, 0.0});
    for (unsigned int i = 0; i < 20; ++i) {
        fcentres[i] = (ico.vertices[ico.faces[i][0]] + ico.vertices[ico.faces[i][1]] + ico.vertices[ico.faces[i][2]]).as_float() / 3.0f;
    }
    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::Scale<float, float> scale;
        scale.setParams (1.0, 0.0);

        morph::vvec<float> data(12, 0.06f);
        morph::vvec<float> data2(20, 0.95f);

        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&ico.vertices);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.01f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);

        // Use a second scatter visual to show the centre of each face, numbered in a different colour
        sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&fcentres);
        sv->setScalarData (&data2);
        sv->radiusFixed = 0.01f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);

        // Triangle visuals for the faces
        morph::ColourMap<float> cm(morph::ColourMapType::Jet);
        for (unsigned int i = 0; i < 20; ++i) {
            std::array<float, 3> colr = cm.convert (i/20.0f);
            auto tv = std::make_unique<morph::TriangleVisual<>> (offset,
                                                                 ico.vertices[ico.faces[i][0]],
                                                                 ico.vertices[ico.faces[i][1]],
                                                                 ico.vertices[ico.faces[i][2]],
                                                                 colr);
            v.bindmodel (tv);
            tv->setAlpha (0.8f);
            tv->finalize();
            v.addVisualModel (tv);
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
