/*
 * Visualize a graph
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GraphVisual.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main (int argc, char** argv)
{
    int rtn = -1;

    morph::Visual v(1024, 768, "Graph", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.backgroundWhite();
    v.lightingEffects();

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        holdVis = a1.empty() ? false : true;
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    try {
        morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::Scale<float> scale;
        scale.setParams (1.0, 0.0);

        std::vector<float> ord =  {0, .1,    .2,    .3,    .4,    .5,    .6,    .7,    .8};
        std::vector<float> data = {0, .1*.1, .2*.2, .3*.3, .4*.4, .5*.5, .6*.6, .7*.7, .8*.8};

        // Create GraphVisual:
        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, offset);
        // Set up the data:
        gv->colourScale.do_autoscale = true;
        gv->zScale.do_autoscale = true;
        gv->ordscale.do_autoscale = true;
        gv->setData (ord, data);
        gv->cm.setType (morph::ColourMapType::Plasma);
        gv->showMarkers = true;
        gv->showLines = true;
        gv->markerColour = {0,.8,1};
        gv->setup();
        // Add the GraphVisual (as a VisualModel*)
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        v.render();
        if (holdVis == true) {
            while (v.readyToFinish == false) {
                glfwWaitEventsTimeout (0.018);
                v.render();
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
