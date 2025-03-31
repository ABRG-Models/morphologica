/*
 * Visualize an Icosahedron using the Geodesic Visual that co-ops the unordered
 * constexpr geodesic function.
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GeodesicVisualCE.h>
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

    morph::Visual v(1024, 768, "(constexpr) Geodesic Polyhedra");
    v.showCoordArrows (true);
    v.lightingEffects (true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::vec<float, 3> step = { 2.2, 0.0, 0.0 };

        auto gv1 = std::make_unique<morph::GeodesicVisualCE<float, 0>> (offset + step * 0, 0.9f);
        v.bindmodel (gv1);
        std::string lbl = std::string("iterations = 0");
        gv1->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
        gv1->finalize();
        v.addVisualModel (gv1);

        auto gv2 = std::make_unique<morph::GeodesicVisualCE<float, 1>> (offset + step * 1, 0.9f);
        v.bindmodel (gv2);
        lbl = std::string("iterations = 1");
        gv2->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
        gv2->finalize();
        v.addVisualModel (gv2);

        auto gv3 = std::make_unique<morph::GeodesicVisualCE<float, 2>> (offset + step * 2, 0.9f);
        v.bindmodel (gv3);
        lbl = std::string("iterations = 2");
        gv3->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
        gv3->finalize();
        v.addVisualModel (gv3);

        auto gv4 = std::make_unique<morph::GeodesicVisualCE<float, 3>> (offset + step * 3, 0.9f);
        v.bindmodel (gv4);
        lbl = std::string("iterations = 4");
        gv4->addLabel (lbl, {0, -1, 0}, morph::TextFeatures(0.06f));
        gv4->finalize();
        v.addVisualModel (gv4);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
