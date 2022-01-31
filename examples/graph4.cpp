/*
 * Visualize a graph on which points are added with time.
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

    morph::Visual v(1024, 768, "Graph", {-0.8,-0.8}, {.1,.1,.1}, 2.0f, 0.01f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.backgroundWhite();
    v.lightingEffects();

    try {
        morph::vVector<float> absc =  {-1.0, -.9, -.8, -.7, -.6, -.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8, .9, 1.0};
        morph::vVector<float> data = absc.pow(3);
        morph::vVector<float> data2 = absc.pow(5);
        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});

        // Optionally change the size of the graph and range of the axes
        gv->setsize (1.33, 1);
        // Optionally change the range of the axes
        gv->setlimits (-1, 1, -1, 1);

        // Set the graphing policy
        gv->policy = morph::stylepolicy::lines; // markers, lines, both, allcolour
        gv->axisstyle = morph::axisstyle::twinax;
        // We 'prepare' two datasets, but won't fill them with data yet. However, we do give the data legend label here.
        gv->prepdata ("Third power", morph::axisside::left);
        gv->prepdata ("Fifth power", morph::axisside::right);

        using morph::unicode;
        gv->ylabel = "f(x) = x" + unicode::toUtf8(unicode::ss3);
        // ylabel2 is the right hand y axis label
        gv->ylabel2 = "f(x) = x" + unicode::toUtf8(unicode::ss5);

        gv->finalize();

        // Add the GraphVisual (as a VisualModel*)
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        size_t rcount = 0;
        size_t idx = 0;
        v.render();
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            // Slowly update the content of the graph
            if (rcount++ % 20 == 0 && idx < absc.size()) {
                // Append to dataset 0
                gv->append (absc[idx], data[idx], 0);
                // Append to dataset 1
                gv->append (absc[idx], data2[idx], 1);
                ++idx;
            }
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
