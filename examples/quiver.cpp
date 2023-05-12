/*
 * Visualize an example quiver field
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/QuiverVisual.h>
#include <morph/vec.h>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
{
    int rtn = -1;

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows
    morph::Visual v(1024, 768, "morph::QuiverVisual", {0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.01f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.showTitle = true;
    // For a white background:
    v.backgroundBlack();
    v.lightingEffects();

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        std::vector<morph::vec<float, 3>> coords(20*20);
        std::vector<morph::vec<float, 3>> quivs(20*20);

        size_t k = 0;
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                float x = 0.1*i;
                float y = 0.1*j;
                // z is some function of x, y
                float z = x * std::exp(-(x*x) - (y*y));
                coords[k] = {x, y, z};
                k++;
            }
        }

        k = 0;
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                if (i > -10 && i < 10 && j > -10 && j < 10) {
                    morph::vec<float> r = coords[k] - coords[k-20];
                    morph::vec<float> g = coords[k] - coords[k-1];
                    // Compute normal and modulate by the 'z' value
                    quivs[k] = r.cross(g)*30.0f*coords[k][2];
                } else {
                    quivs[k] = {0.0f, 0.0f, 0.0f};
                }
                k++;
            }
        }
        auto vmp = std::make_unique<morph::QuiverVisual<float>>(v.shaders, &coords, offset, &quivs, morph::ColourMapType::MonochromeGreen);
        vmp->finalize();
        v.addVisualModel (vmp);

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
