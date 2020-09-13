#include "morph/Visual.h"
#include "morph/VisualDataModel.h"
#include "morph/HexGridVisual.h"
#include "morph/HexGrid.h"
#include "morph/ReadCurves.h"
#include "morph/tools.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>
#include "morph/Scale.h"
#include "morph/Vector.h"

using namespace std;
using morph::Visual;
using morph::VisualDataModel;
using morph::HexGrid;
using morph::HexGridVisual;
using morph::Tools;
using morph::HexDomainShape;
using morph::ReadCurves;
using morph::Scale;
using morph::Vector;

int main()
{
    int rtn = -1;

    Visual v(800,600,"Test window");
    // Set up the near and far cutoff distances for rendering objects
    v.zNear = 0.001;
    v.zFar = 50;
    // Set up a field of view (in degrees) for the camera
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked = false;
    // Whole-scene offsetting uses two methods - one to set the depth at which the object is drawn
    v.setZDefault (-5.0f);
    // and one to set the x/y offset. Try pressing 'z' in the app window to see what the current sceneTrans is
    v.setSceneTransXY (0.0f, 0.0f);
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;

    try {
        string pwd = Tools::getPwd();
        string curvepath = "./tests/trial.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "./../tests/trial.svg";
        }
        ReadCurves r(curvepath);

        HexGrid hg(0.01, 3, 0, HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        cout << hg.extent() << endl;

        cout << "Number of hexes in grid:" << hg.num() << endl;
        cout << "Last vector index:" << hg.lastVectorIndex() << endl;

        if (hg.num() != 1604) {
            rtn = -1;
        }

        vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*std::sin(10*hg.d_x[hi]); // Range 0->1
        }
        cout << "Created " << data.size() << " floats in data" << endl;

        Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        unsigned int gridId = v.addVisualModel (new HexGridVisual<float>(v.shaderprog, &hg, offset, &data));
        cout << "Added HexGridVisual with gridId " << gridId << endl;

        // Divide existing scale by 10:
        float newGrad = static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->zScale.getParams(0)/10.0;
        // Set this in a new zscale object:
        Scale<float> zscale;
        zscale.setParams (newGrad, 0);
        // And set it back into the visual model:
        static_cast<VisualDataModel<float>*>(v.getVisualModel(gridId))->setZScale (zscale);

        v.render();

        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const exception& e) {
        cerr << "Caught exception reading trial.svg: " << e.what() << endl;
        cerr << "Current working directory: " << Tools::getPwd() << endl;
        rtn = -1;
    }


    return rtn;
}
