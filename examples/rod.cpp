/*
 * Visualize a Rod
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/RodVisual.h>
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

    morph::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        morph::vec<float, 3> start = { 0, 0, 0 };
        morph::vec<float, 3> end = { 0.25, 0, 0 };

        morph::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 };
        morph::vec<float, 3> colour2 = { 0.0, 0.9, 0.4 };

        std::unique_ptr<morph::VisualModel<>> rvm = std::make_unique<morph::RodVisual<>> (offset, start, end, 0.1f, colour1, colour2);
        v.bindmodel (rvm);
        rvm->finalize();
        v.addVisualModel (rvm);

        morph::vec<float, 3> start2 = { -0.1, 0.2, 0.6 };
        morph::vec<float, 3> end2 = { 0.2, 0.4, 0.6 };
        // You can reuse the unique_ptr rvm, once you've transferred ownership with v.addVisualModel (rvm)
        rvm = std::make_unique<morph::RodVisual<>>(offset, start2, end2, 0.05f, colour2);
        v.bindmodel (rvm);
        rvm->finalize();
        v.addVisualModel (rvm);

        v.render();
        while (v.readyToFinish == false) {
            v.waitevents (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
