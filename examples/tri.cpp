/*
 * Visualize a triangle
 */
#include <morph/Visual.h>
#include <morph/TriangleVisual.h>
#include <morph/vec.h>
#include <iostream>

int main (int argc, char** argv)
{
    int rtn = -1;

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows
    morph::Visual v(1024, 768, "Visualization", {0,0}, {.5,.5,.5}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = false;
    v.coordArrowsInScene = true;
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        morph::vec<float, 3> c1 = { 0, 0, 0 };
        morph::vec<float, 3> c2 = { 0.25, 0, 0 };
        morph::vec<float, 3> c3 = { 0.0, 0.3, 0 };

        morph::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 };

        auto tv = std::make_unique<morph::TriangleVisual> (v.shaders, offset, c1, c2, c3, colour1);
        v.addVisualModel (tv);

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
