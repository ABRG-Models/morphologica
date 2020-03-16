/*
 * Visualize a quiver field
 */
#include "Visual.h"
using morph::Visual;
#include "ColourMap.h"
using morph::ColourMapType;
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

        vector<array<float, 3>> coords;
        coords.push_back ({0, 0,   0});
        coords.push_back ({1, 1,   0});
        coords.push_back ({2, 0,   0});
        coords.push_back ({1, 0.8, 0});
        coords.push_back ({2, 0.5, 0});

        vector<array<float, 3>> quivs;
        quivs.push_back ({0.3,   0.4,  0});
        quivs.push_back ({0.1,   0.2,  0});
        quivs.push_back ({-0.1,  0,    0});
        quivs.push_back ({-0.04, 0.05, 0});
        quivs.push_back ({0.3,  -0.1,  0});

        unsigned int visId = v.addQuiverVisual (&coords, offset, &quivs, ColourMapType::Cividis);

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
