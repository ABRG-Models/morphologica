/*
 * Visualize a quiver field
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/QuiverVisual.h>
#include <morph/ScatterVisual.h>
#include <morph/vec.h>
#include <morph/scale.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main (int argc, char** argv)
{
    using namespace std;

    int rtn = -1;

    morph::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows (true);
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
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        vector<morph::vec<float, 3>> coords;
        coords.push_back ({0, 0,   0});
        coords.push_back ({1, 1,   0});
        coords.push_back ({2, 0,   0});
        coords.push_back ({1, 0.8, 0});
        coords.push_back ({2, 0.5, 0});

        vector<morph::vec<float, 3>> quivs;
        quivs.push_back ({0.3,   0.4,  0});
        quivs.push_back ({0.1,   0.2,  0.1});
        quivs.push_back ({-0.1,  0,    0});
        quivs.push_back ({-0.04, 0.05, -.2});
        quivs.push_back ({0.3,  -0.1,  0});

        auto qvp = std::make_unique<morph::QuiverVisual<float>> (&coords, offset, &quivs, morph::ColourMapType::Cividis);
        v.bindmodel (qvp);
        qvp->finalize();
        unsigned int visId = v.addVisualModelId (qvp);
        cout << "Added Visual with visId " << visId << endl;

        offset = { 0.0, 0.1, 0.0 };
        morph::scale<float> scale;
        scale.setParams (1.0, 0.0);

        vector<morph::vec<float, 3>> points;
        points.push_back ({0,0,0});
        points.push_back ({1,1,0});
        points.push_back ({2,2.2,0});
        points.push_back ({3,2.8,0});
        points.push_back ({4,3.9,0});
        vector<float> data = {0.1, 0.2, 0.5, 0.6, 0.95};

        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&points);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.03f;
        sv->colourScale = scale;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->finalize();
        morph::ScatterVisual<float>* visPtr = v.addVisualModel (sv);

        v.render();
        // 10 seconds of viewing the quivers
        if (holdVis == true) {
            for (size_t ti = 0; ti < (size_t)std::round(10.0/0.018); ++ti) {
                glfwWaitEventsTimeout(0.018);
                v.render();
            }
        }

        std::cout << "Remove model " << visId << " (the quivers)" << std::endl;
        v.removeVisualModel (visId);

        // 10 seconds of viewing the remaining scatter plot
        if (holdVis == true) {
            for (size_t ti = 0; ti < (size_t)std::round(10.0/0.018); ++ti) {
                glfwWaitEventsTimeout(0.018);
                v.render();
            }
        }

        std::cout << "Remove scatter model with a pointer" << std::endl;
        v.removeVisualModel (visPtr);

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
