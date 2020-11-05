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
    morph::Visual v(1024, 768, "Visualization", {0,0,0}, {1,1,1}, 0.5f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    try {
        morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };

        //morph::Vector<float, 3> start = { -0.1, -0.1, -0.3 };
        //morph::Vector<float, 3> end = { 0.3, 0.3, 0.3 };
        morph::Vector<float, 3> start = { 0.0, 0.0, 0.0 };
        morph::Vector<float, 3> end = { 0.5, 0.0, 0.0 };

        morph::Vector<float, 3> colour1 = { 1.0, 0.0, 0.0 };

        unsigned int visId = v.addVisualModel (new morph::RodVisual (v.shaderprog, offset, start, end, 0.05f, colour1));

        std::cout << "Added RodVisual with visId " << visId << std::endl;

        v.render();
        if (holdVis == true) {
            while (v.readyToFinish == false) {
                glfwWaitEventsTimeout (0.018);
                v.render();
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
