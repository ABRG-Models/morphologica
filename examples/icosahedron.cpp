/*
 * Visualize an Icosahedron
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/IcosaVisual.h>
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

    morph::Visual v(1024, 768, "Icosahedron");
    v.showCoordArrows (true);
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 };

        auto iv = std::make_unique<morph::IcosaVisual<>> (offset, 0.9f, colour1);
        v.bindmodel (iv);
        iv->finalize();
        v.addVisualModel (iv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
