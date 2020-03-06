/*
 * Visualize a test surface
 */
#include "Visual.h"
using morph::Visual;
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
//#include "ColourMap.h"

using namespace std;

int main (int argc, char** argv)
{
    int rtn = -1;

    Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;


    try {
        array<float, 3> offset = { 0.0, 0.0, 0.0 };
        array<float, 2> scale = { 1.0, 0.0};

        vector<array<float, 3>> points;
        vector<float> data;
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

        //vector<float> data = {0.1, 0.7, 0.1, 0.9, 0.3, 0.8, 0.1, 0.1, 0.5, 0.1};

        unsigned int visId = v.addPointRowsVisual (&points, offset, data, scale, morph::ColourMapType::Jet);
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
