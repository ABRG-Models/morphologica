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

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "morph::QuiverVisual");
    v.zNear = 0.001;
    v.showCoordArrows (true);
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
        auto vmp = std::make_unique<morph::QuiverVisual<float>>(&coords, offset, &quivs, morph::ColourMapType::MonochromeGreen);
        v.bindmodel (vmp);
        vmp->quiver_length_gain = 0.4f; // Scale the length of the quivers on screen
        vmp->quiver_thickness_gain = 0.05f; // Scale thickness of the quivers
        // vmp->fixed_quiver_thickness = 0.003f; // Also possible to request a fixed thickness
        vmp->shapesides = 24; // Default is 12, this makes the graphics look nicer - it
                              // controls how many triangles go into each cone, rod and
                              // sphere in the quivers.
        vmp->finalize();
        v.addVisualModel (vmp);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
