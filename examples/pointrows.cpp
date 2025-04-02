/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#ifdef MESH
#include <morph/PointRowsMeshVisual.h>
#else
#include <morph/PointRowsVisual.h>
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
#ifdef MESH
    morph::Visual v(1024, 768, "morph::PointRowsMeshVisual");
#else
    morph::Visual v(1024, 768, "morph::PointRowsVisual");
#endif
    v.zNear = 0.001;
    v.showCoordArrows (true);
    v.lightingEffects (true);

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::scale<float> scale;
        scale.setParams (1.0, 0.0);

        std::vector<morph::vec<float, 3>> points;
        std::vector<float> data; // copy points[:][2] into data
        points.push_back ({ 0, 0,   0.1 }); data.push_back(points.back()[2]);
        points.push_back ({ 0, 2,   0.7 }); data.push_back(points.back()[2]);
        points.push_back ({ 0, 4,   0.1 }); data.push_back(points.back()[2]);

        points.push_back ({ 1, 0,   0.9  }); data.push_back(points.back()[2]);
        points.push_back ({ 1, 1,   0.3  }); data.push_back(points.back()[2]);
        points.push_back ({ 1, 2.5, 0.8  }); data.push_back(points.back()[2]);
        points.push_back ({ 1, 4,   0.1  }); data.push_back(points.back()[2]);

        points.push_back ({ 2, 0,   0.1 }); data.push_back(points.back()[2]);
        points.push_back ({ 2, 2.1, 0.5 }); data.push_back(points.back()[2]);
        points.push_back ({ 2, 2.7, 0.7 }); data.push_back(points.back()[2]);
        points.push_back ({ 2, 2.9, 0.3 }); data.push_back(points.back()[2]);
        points.push_back ({ 2, 4,   0.1 }); data.push_back(points.back()[2]);

#ifdef MESH
        auto prmv = std::make_unique<morph::PointRowsMeshVisual<float>>(&points, offset, &data, scale, morph::ColourMapType::Twilight,
                                                                        0.0f, 1.0f, 1.0f, 0.04f, morph::ColourMapType::Jet, 0.0f, 1.0f, 1.0f, 0.1f);
        v.bindmodel (prmv);
        prmv->finalize();
        v.addVisualModel (prmv);
#else
        auto prv = std::make_unique<morph::PointRowsVisual<float>>(&points, offset, &data, scale, morph::ColourMapType::Twilight);
        v.bindmodel (prv);
        prv->finalize();
        v.addVisualModel (prv);
#endif

        v.render();
        while (v.readyToFinish() == false) {
            v.waitevents (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }


    return rtn;
}
