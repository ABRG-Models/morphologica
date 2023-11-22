/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "morph::ScatterVisual", {0,0}, {1,1,1}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    v.showTitle = true;
    // Blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};
    v.lightingEffects();

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::Scale<float> scale;
        scale.setParams (1.0, 0.0);

        // Note use of morph::vvecs here, which can be passed into
        // VisualDataModel::setDataCoords(std::vector<vec<float>>* _coords)
        // and setScalarData(const std::vector<T>* _data)
        // This is possible because morph::vvec derives from std::vector.
        morph::vvec<morph::vec<float, 3>> points(20*20);
        morph::vvec<float> data(20*20);
        size_t k = 0;
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                float x = 0.1*i;
                float y = 0.1*j;
                // z is some function of x, y
                float z = x * std::exp(-(x*x) - (y*y));
                points[k] = {x, y, z};
                data[k] = z;
                k++;
            }
        }

        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&points);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.03f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->finalize();
        v.addVisualModel (sv);

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
