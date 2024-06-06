/*
 * Visualize an Icosahedron
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GeodesicVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Geodesic Polyhedron");
    v.showCoordArrows = true;
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::vec<float, 3> step = { 2.2, 0.0, 0.0 };

        morph::ColourMap<float> cmap (morph::ColourMapType::Jet);

        int imax = 7;
        float imax_mult = 1.0f/static_cast<float>(imax);
        for (int i = 0; i < imax; ++i) {
            auto gv1 = std::make_unique<morph::GeodesicVisual<>> (offset + step * i, 0.9f, cmap.convert(i*imax_mult));
            v.bindmodel (gv1);
            gv1->iterations = i;
            gv1->finalize();
            std::string lbl = std::string("iterations = ") + std::to_string(i);
            gv1->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
            v.addVisualModel (gv1);
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
