/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/vVector.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main (int argc, char** argv)
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
        morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::Scale<float> scale;
        scale.setParams (1.0, 0.0);

        // Note use of morph::vVectors here, which can be passed into
        // VisualDataModel::setDataCoords(std::vector<Vector<float>>* _coords)
        // and setScalarData(const std::vector<T>* _data)
        // This is possible because morph::vVector derives from std::vector.
        morph::vVector<morph::Vector<float, 3>> points(20*20);
        morph::vVector<float> data(20*20);
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

        morph::ScatterVisual<float>* sv = new morph::ScatterVisual<float> (v.shaderprog, offset);
        sv->setDataCoords (&points);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.03f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->finalize();
        unsigned int visId = v.addVisualModel (sv);

        std::cout << "Added Visual with visId " << visId << std::endl;

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
