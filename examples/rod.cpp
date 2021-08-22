/*
 * Visualize a Rod
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/RodVisual.h>
#include <morph/Vector.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
{
    int rtn = -1;

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows
    morph::Visual v(1024, 768, "Visualization", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };

        morph::Vector<float, 3> start = { 0, 0, 0 };
        morph::Vector<float, 3> end = { 0.25, 0, 0 };

        morph::Vector<float, 3> colour1 = { 1.0, 0.0, 0.0 };
        morph::Vector<float, 3> colour2 = { 0.0, 0.9, 0.4 };

        unsigned int visId = v.addVisualModel (new morph::RodVisual (v.shaderprog, offset, start, end, 0.1f, colour1, colour2));

        std::cout << "Added RodVisual with visId " << visId << std::endl;
#if 1
        morph::Vector<float, 3> start2 = { -0.1, 0.2, 0.6 };
        morph::Vector<float, 3> end2 = { 0.2, 0.4, 0.6 };

        visId = v.addVisualModel (new morph::RodVisual (v.shaderprog, offset, start2, end2, 0.05f, colour2));

        std::cout << "Added RodVisual with visId " << visId << std::endl;
#endif
        v.render();
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
