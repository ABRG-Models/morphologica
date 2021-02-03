/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#ifdef MESH
# include <morph/QuadsMeshVisual.h>
#else
# include <morph/QuadsVisual.h>
#endif
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main (int argc, char** argv)
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.lightingEffects (true);

    try {
        morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::Scale<float> scale;
        scale.setParams (1.0, 0.0);

        std::vector<std::array<float, 12>> surfBoxes;

        std::array<float,12> box1 = { 0,0,0,
                                      0.5,1,0.5,
                                      1.5,1,0.5,
                                      2,0,0 };

        std::array<float,12> box2 = { 0.5,1,0.5,
                                      0,2,0,
                                      2,2,0,
                                      1.5,1,0.5 };

        std::array<float,12> box3 = { 4,0,0,
                                      3.5,1,0.5,
                                      5,1,0.5,
                                      4.5,0,0 };

        std::array<float,12> box4 = { 3.5,1,0.5,
                                      4,2,0,
                                      4.5,2,0,
                                      5,1,0.5};

        surfBoxes.push_back (box1);
        surfBoxes.push_back (box2);
        surfBoxes.push_back (box3);
        surfBoxes.push_back (box4);

        std::vector<float> data = {0.1, 0.2, 0.5, 0.95};

#ifdef MESH
        unsigned int visId = v.addVisualModel (new morph::QuadsMeshVisual<float> (v.shaderprog, &surfBoxes, offset, &data, scale, morph::ColourMapType::Plasma));
#else
        unsigned int visId = v.addVisualModel (new morph::QuadsVisual<float> (v.shaderprog, &surfBoxes, offset, &data, scale, morph::ColourMapType::Monochrome));
#endif

        std::cout << "Added Visual with visId " << visId << std::endl;

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
