/*
 * Visualize a test surface
 */
#include "Visual.h"
using morph::Visual;
#include "ColourMap.h"
using morph::ColourMapType;
#include "ScatterVisual.h"
using morph::ScatterVisual;
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

using namespace std;

int main (int argc, char** argv)
{
    int rtn = -1;

    Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows = true;
    // Blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};

    bool holdVis = false;
    if (argc > 1) {
        string a1(argv[1]);
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << endl;

    try {
        array<float, 3> offset = { 0.0, 0.0, 0.0 };
        array<float, 2> scale = { 1.0, 0.0};

        vector<array<float, 3>> points;
        points.push_back ({0,0,0});
        points.push_back ({1,1,0});
        points.push_back ({2,2.2,0});
        points.push_back ({3,2.8,0});
        points.push_back ({4,3.9,0});

#if 0
        vector<float> data;
#else
        vector<float> data = {0.1, 0.2, 0.5, 0.6, 0.95};
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
