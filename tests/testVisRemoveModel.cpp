/*
 * Visualize a quiver field
 */
#include "Visual.h"
using morph::Visual;
#include "ColourMap.h"
using morph::ColourMapType;
#include "QuiverVisual.h"
using morph::QuiverVisual;
#include "ScatterVisual.h"
#include "Vector.h"
using morph::Vector;
#include "Scale.h"
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
    // For a white background:
    v.backgroundWhite();

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

        vector<Vector<float, 3>> coords;
        coords.push_back ({0, 0,   0});
        coords.push_back ({1, 1,   0});
        coords.push_back ({2, 0,   0});
        coords.push_back ({1, 0.8, 0});
        coords.push_back ({2, 0.5, 0});

        vector<Vector<float, 3>> quivs;
        quivs.push_back ({0.3,   0.4,  0});
        quivs.push_back ({0.1,   0.2,  0.1});
        quivs.push_back ({-0.1,  0,    0});
        quivs.push_back ({-0.04, 0.05, -.2});
        quivs.push_back ({0.3,  -0.1,  0});

        unsigned int visId = v.addVisualModel (new morph::QuiverVisual<float> (v.shaderprog, &coords, offset, &quivs, ColourMapType::Cividis));

        cout << "Added Visual with visId " << visId << endl;

        offset = { 0.0, 0.1, 0.0 };
        morph::Scale<float> scale;
        scale.setParams (1.0, 0.0);

        vector<Vector<float, 3>> points;
        points.push_back ({0,0,0});
        points.push_back ({1,1,0});
        points.push_back ({2,2.2,0});
        points.push_back ({3,2.8,0});
        points.push_back ({4,3.9,0});
        vector<float> data = {0.1, 0.2, 0.5, 0.6, 0.95};

        unsigned int visId_s = v.addVisualModel (new morph::ScatterVisual<float> (v.shaderprog, &points, offset, &data, 0.03f, scale, ColourMapType::Plasma));

        cout << "Added Visual with visId " << visId_s << endl;

        v.render();
        // 10 seconds of viewing
        if (holdVis == true) {
            for (size_t ti = 0; ti < (size_t)std::round(10.0/0.018); ++ti) {
                glfwWaitEventsTimeout(0.018);
                v.render();
            }
        }

        v.removeVisualModel (visId);

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
