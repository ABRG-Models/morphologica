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

        points.push_back ({ 0, 0,   0.1 });
        points.push_back ({ 0, 2,   0   });
        points.push_back ({ 0, 4,   0.1 });

        points.push_back ({ 1, 0,   0.1  });
        points.push_back ({ 1, 1,   0    });
        points.push_back ({ 1, 2.5, 0    });
        points.push_back ({ 1, 4,   0.1  });

        points.push_back ({ 2, 0,   0.1 });
        points.push_back ({ 2, 2,   0   });
        points.push_back ({ 2, 4,   0.1 });

        vector<float> data = {0.1, 0.2, 0.8, 0.1, 0.2, 0.5, 0.8, 0.8, 0.99, 0.9};

        unsigned int visId = v.addPointRowsVisual (&points, offset, data, scale, morph::ColourMapType::Monochrome);
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
