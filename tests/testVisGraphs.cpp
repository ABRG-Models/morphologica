/*
 * Visualize a graph
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/GraphVisual.h>
#include <morph/Scale.h>
#include <morph/Vector.h>
#include <morph/vVector.h>
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
        morph::vVector<float> ord =  {-.5, -.4, -.3, -.2, -.1, 0, .1,    .2,    .3,    .4,    .5,    .6,    .7,    .8};
        morph::vVector<float> data = ord.pow(3);

        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});
        gv->setdata (ord, data);
        gv->linecolour = {1.0, 0.0, 0.0};
        gv->markerstyle = morph::markerstyle::triangle;
        gv->markercolour = {0.0, 0.0, 1.0};
        gv->axisstyle = morph::axisstyle::L;
        gv->setup();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {1.2,0,0});
        morph::vVector<float> data2 = ord.pow(2);
        gv->setdata (ord, data2);
        gv->linecolour = {0.0, 0.0, 1.0};
        gv->markerstyle = morph::markerstyle::hexagon;
        gv->markercolour = {0.0, 0.0, 0.0};
        gv->axisstyle = morph::axisstyle::box;
        gv->setup();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,-1.2,0});
        morph::vVector<float> data3 = ord.pow(4);
        gv->setdata (ord, data3);
        gv->linecolour = {0.0, 1.0, 0.0};
        gv->markerstyle = morph::markerstyle::circle;
        gv->markercolour = {0.0, 1.0, 0.0};
        gv->markergap = 0.0f;
        gv->axisstyle = morph::axisstyle::boxfullticks;
        gv->setup();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {1.2,-1.2,0});
        morph::vVector<float> data4 = ord.pow(5);
        gv->setdata (ord, data4);
        gv->linecolour = {0.0, 0.0, 1.0};
        gv->markerstyle = morph::markerstyle::none;
        gv->markergap = 0.0f;
        gv->axisstyle = morph::axisstyle::cross;
        gv->setup();
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
