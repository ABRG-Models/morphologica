/*
 * Visualize a test surface
 */
#include "morph/Visual.h"
using morph::Visual;
#include "morph/ColourMap.h"
using morph::ColourMapType;
#ifdef MESH
#include "morph/PointRowsMeshVisual.h"
using morph::PointRowsMeshVisual;
#else
#include "morph/PointRowsVisual.h"
using morph::PointRowsVisual;
#endif
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

    Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.lightingEffects (true);

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
        vector<float> data; // copy points[:][2] into data
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
        unsigned int visId = v.addVisualModel (new PointRowsMeshVisual<float> (v.shaderprog, &points, offset, &data, scale, ColourMapType::Twilight));
#else
        unsigned int visId = v.addVisualModel (new PointRowsVisual<float> (v.shaderprog, &points, offset, &data, scale, ColourMapType::Twilight));
#endif
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
