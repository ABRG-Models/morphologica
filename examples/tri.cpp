/*
 * Visualize a triangle
 */
#include <morph/Visual.h>
#include <morph/TriangleVisual.h>
#include <morph/vec.h>
#include <iostream>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.coordArrowsInScene (true);
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

        auto tv = std::make_unique<morph::TriangleVisual<>> (offset, c1, c2, c3, colour1);
        v.bindmodel (tv);
        tv->finalize();
        v.addVisualModel (tv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
