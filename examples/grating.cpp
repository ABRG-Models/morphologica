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

int main (int ac, char** av)
{
    int rtn = -1;
    morph::Visual v(1024, 768, "Grating");

    int t0 = 0; // start time
    if (ac > 1) { t0 = std::atoi (av[1]); }

    constexpr bool move_stuff = true;

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        auto rvm = std::make_unique<morph::GratingVisual<>> (offset);
        v.bindmodel (rvm);
        rvm->v_front = { 0.02f, 0.0f };
        rvm->t = t0;
        rvm->finalize();
        auto rvmp = v.addVisualModel (rvm);

        if constexpr (move_stuff == true) {
            int count = 0;
            while (count++ < 7000) {
                v.wait (0.04);
                v.render();
                rvmp->t += 1;
                rvmp->reinit();
            }
            v.keepOpen();
        } else {
            v.keepOpen();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
