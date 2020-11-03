/*
 * Visualize a quiver field
 */
#include "morph/Visual.h"
using morph::Visual;
#include "morph/ColourMap.h"
using morph::ColourMapType;
#include "morph/QuiverVisual.h"
using morph::QuiverVisual;
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

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows
    Visual v(1024, 768, "Visualization", {0,0,0}, {1,1,1}, 0.5f);
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

        unsigned int visId = v.addVisualModel (new QuiverVisual<float> (v.shaderprog, &coords, offset, &quivs, ColourMapType::Cividis));

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
