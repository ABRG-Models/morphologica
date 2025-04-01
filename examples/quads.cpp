/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#ifdef MESH
# include <morph/QuadsMeshVisual.h>
#else
# include <morph/QuadsVisual.h>
#endif
#include <morph/scale.h>
#include <morph/vec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows (true);
    v.lightingEffects (true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::scale<float> scale1;
        scale1.setParams (1.0, 0.0);

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
        auto qmv = std::make_unique<morph::QuadsMeshVisual<float>> (&surfBoxes, offset, &data, scale1, morph::ColourMapType::Plasma);
        v.bindmodel (qmv);
        qmv->finalize();
        v.addVisualModel (qmv);
#else
        auto qv = std::make_unique<morph::QuadsVisual<float>> (&surfBoxes, offset, &data, scale1, morph::ColourMapType::Monochrome);
        v.bindmodel (qv);
        qv->finalize();
        v.addVisualModel (qv);
#endif
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
