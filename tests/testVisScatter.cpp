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

    Visual v(1024, 768, "Visualization", {0,0}, {1,1,1}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    // Blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};
    v.lightingEffects();

    bool holdVis = false;
    if (argc > 1) {
        string a1(argv[1]);
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << endl;

    try {
        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        Scale<float> scale;
        scale.setParams (1.0, 0.0);

        vector<Vector<float, 3>> points;
        points.push_back ({0,0,0});
        points.push_back ({.1,.1,0});
        points.push_back ({.2,.22,0});
        points.push_back ({.3,.28,0});
        points.push_back ({.4,.39,0});
        points.push_back ({.6,.55,0});
        points.push_back ({.65,.7,0});
        points.push_back ({.76,.8,0});
        points.push_back ({.9,.9,0});

#if 0
        vector<float> data;
#else
        vector<float> data = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
#endif

        unsigned int visId = v.addVisualModel (new ScatterVisual<float> (v.shaderprog, &points, offset, &data, 0.03f, scale, ColourMapType::Plasma));

        cout << "Added Visual with visId " << visId << endl;

        v.render();
        if (holdVis == true) {
            while (v.readyToFinish == false) {
                glfwWaitEventsTimeout (0.018);
                v.render();
            }
        }

    } catch (const exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
        rtn = -1;
    }

    return rtn;
}
