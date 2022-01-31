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

    try {
        morph::vVector<float> absc =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};

        float step = 1.4f;
        float row2 = 1.2f;

        morph::DatasetStyle ds;

        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});
        morph::vVector<float> data = absc.pow(3);

        ds.linecolour =  {1.0, 0.0, 0.0};
        ds.linewidth = 0.015f;
        ds.markerstyle = morph::markerstyle::triangle;
        ds.markercolour = {0.0, 0.0, 1.0};
        gv->setdata (absc, data, ds);

        gv->axisstyle = morph::axisstyle::L;

        using morph::unicode;
        // Set xlabel to include the greek character alpha:
        gv->xlabel = "Include unicode symbols like this: " + unicode::toUtf8 (unicode::alpha);
        // A gamma - using raw unicode here instead of unicode::gamma
        gv->ylabel = "Unicode for Greek gamma is 0x03b3: " + morph::unicode::toUtf8 (0x03b3);

        gv->setthickness (0.001f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {step,0,0});
        morph::vVector<float> data2 = absc.pow(2);
        ds.linecolour = {0.0, 0.0, 1.0};
        ds.markerstyle = morph::markerstyle::hexagon;
        ds.markercolour = {0.0, 0.0, 0.0};
        gv->setdata (absc, data2, ds);
        gv->axisstyle = morph::axisstyle::box;
        gv->ylabel = "mm";
        gv->xlabel = "Abscissa (notice that mm is not rotated)";
        gv->setthickness (0.005f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,-row2,0});
        morph::vVector<float> data3 = absc.pow(4);
        gv->setsize (1,0.8);
        ds.linecolour = {0.0, 1.0, 0.0};
        ds.markerstyle = morph::markerstyle::circle;
        ds.markercolour = {0.0, 0.0, 1.0};
        ds.markersize = 0.02f;
        ds.markergap = 0.0f;
        gv->setdata (absc, data3, ds);
        gv->axisstyle = morph::axisstyle::boxfullticks;
        gv->tickstyle = morph::tickstyle::ticksin;
        gv->ylabel = "mmi";
        gv->xlabel = "mmi is just long enough to be rotated";
        gv->setthickness (0.001f);
        gv->finalize();
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {step,-row2,0});
        absc.resize(1000, 0.0f);
        for (int i = 0; i < 1000; ++i) {
            absc[i] = static_cast<float>(i-500) * 0.01f;
        }
        morph::vVector<float> data4 = absc.pow(5);
        gv->setsize (1,0.8);
        ds.linecolour = {0.0, 0.0, 1.0};
        ds.markerstyle = morph::markerstyle::none;
        ds.markergap = 0.0f;
        gv->setdata (absc, data4, ds);
        gv->axisstyle = morph::axisstyle::cross;
        gv->setthickness (0.002f);
        gv->finalize();
        gv->twodimensional = false;
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        v.render();
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
