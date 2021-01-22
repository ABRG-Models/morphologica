#include <utility>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>

#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/tools.h>

int main()
{
    int rtn = -1;

    morph::Visual v(1600, 1000, "morph::Visual", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.0f);
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
    v.showCoordArrows = true;
    v.showTitle = true;
    v.coordArrowsInScene = false;
    v.backgroundWhite();
    v.lightingEffects();
    v.addLabel ("Each object is derived from morph::VisualModel", {0.005f, -0.02f, 0.0f});
    v.addLabel ("This is a morph::CoordArrows object", {0.03f, -0.23f, 0.0f});
    v.addLabel ("This is a\nmorph::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    try {
        std::string pwd = morph::Tools::getPwd();
        std::string curvepath = "./tests/trial.svg";
        if (pwd.substr(pwd.length()-11) == "build/tests") {
            curvepath = "./../tests/trial.svg";
        }
        morph::ReadCurves r(curvepath);

        morph::HexGrid hg(0.01, 3, 0, morph::HexDomainShape::Boundary);
        hg.setBoundary (r.getCorticalPath());

        std::cout << hg.extent() << std::endl;

        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 1604) {
            rtn = -1;
        }

        std::vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*std::sin(10*hg.d_x[hi]); // Range 0->1
        }
        std::cout << "Created " << data.size() << " floats in data" << std::endl;

        morph::Vector<float, 3> offset = { 0.0, -0.05, 0.0 };
        unsigned int gridId = v.addVisualModel (new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, &hg, offset, &data));
        std::cout << "Added HexGridVisual with gridId " << gridId << std::endl;

        // Divide existing scale by 10:
        float newGrad = static_cast<morph::VisualDataModel<float>*>(v.getVisualModel(gridId))->zScale.getParams(0)/10.0;
        // Set this in a new zscale object:
        morph::Scale<float> zscale;
        zscale.setParams (newGrad, 0);
        // And set it back into the visual model:
        static_cast<morph::VisualDataModel<float>*>(v.getVisualModel(gridId))->setZScale (zscale);

        v.render();

        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << morph::Tools::getPwd() << std::endl;
        rtn = -1;
    }


    return rtn;
}
