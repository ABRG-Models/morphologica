/*
 * Visualize a Grating
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GratingVisual.h>
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
    morph::Visual v(1024, 768, "Grating");

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        auto rvm = std::make_unique<morph::GratingVisual<>> (offset);
        v.bindmodel (rvm);
        rvm->finalize();
        v.addVisualModel (rvm);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
