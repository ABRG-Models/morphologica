/*
 * Visualize a test surface
 */
#include "morph/Visual.h"
using morph::Visual;
#include "morph/ColourMap.h"
using morph::ColourMapType;
#include "morph/ScatterVisual.h"
using morph::ScatterVisual;
#include "morph/Scale.h"
using morph::Scale;
#include "morph/Vector.h"
using morph::Vector;
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

using namespace std;

int main (int argc, char** argv)
{
    int rtn = -1;

    Visual v(1024, 768, "morph::ScatterVisual", {0,0}, {1,1,1}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    v.showTitle = true;
    // Blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};
    v.lightingEffects();

    try {
        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        Scale<float> scale;
        scale.setParams (1.0, 0.0);

        std::vector<morph::Vector<float, 3>> points(20*20);
        vector<float> data(20*20);
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

        unsigned int visId = v.addVisualModel (new ScatterVisual<float> (v.shaderprog, &points, offset, &data, 0.03f, scale, ColourMapType::Plasma));

        cout << "Added Visual with visId " << visId << endl;

        v.render();
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
