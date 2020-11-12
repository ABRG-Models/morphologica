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
        morph::vVector<float> absc =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};

        float step = 1.4f;
        float row2 = 1.2f;

        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});
        morph::vVector<float> data = absc.pow(3);
        gv->setdata (absc, data);
        gv->linecolour = {1.0, 0.0, 0.0};
        gv->markerstyle = morph::markerstyle::triangle;
        gv->markercolour = {0.0, 0.0, 1.0};
        gv->axisstyle = morph::axisstyle::L;
        gv->xlabel = "The x axis";
        gv->setthickness (0.001f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {step,0,0});
        morph::vVector<float> data2 = absc.pow(2);
        gv->setdata (absc, data2);
        gv->linecolour = {0.0, 0.0, 1.0};
        gv->markerstyle = morph::markerstyle::hexagon;
        gv->markercolour = {0.0, 0.0, 0.0};
        gv->axisstyle = morph::axisstyle::box;
        gv->ylabel = "mm";
        gv->xlabel = "Abscissa (notice that mm is not rotated)";
        gv->setthickness (0.005f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,-row2,0});
        morph::vVector<float> data3 = absc.pow(4);
        gv->setsize (1,0.8);
        gv->setdata (absc, data3);
        gv->linecolour = {0.0, 1.0, 0.0};
        gv->markerstyle = morph::markerstyle::circle;
        gv->markercolour = {0.0, 1.0, 0.0};
        gv->markergap = 0.0f;
        gv->axisstyle = morph::axisstyle::boxfullticks;
        gv->tickstyle = morph::tickstyle::ticksin;
        gv->ylabel = "mmi";
        gv->xlabel = "mmi is just long enough to be rotated";
        gv->setthickness (0.01f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {step,-row2,0});
        morph::vVector<float> data4 = absc.pow(5);
        gv->setsize (1,0.8);
        gv->setdata (absc, data4);
        gv->linecolour = {0.0, 0.0, 1.0};
        gv->markerstyle = morph::markerstyle::none;
        gv->markergap = 0.0f;
        gv->axisstyle = morph::axisstyle::cross;
        gv->setthickness (0.05f);
        gv->finalize();
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
