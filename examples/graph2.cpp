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

    morph::Visual v(1024, 768, "Graph", {-0.8,-0.8}, {.1,.1,.1}, 2.0f, 0.01f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.backgroundWhite();
    v.lightingEffects();

    // Optionally (at compile time) change the range of the axes:
    static constexpr bool change_axes_range = false;

    // Optionally (compile time) modify the features of the graph by first creating a DatasetStyle object
    static constexpr bool modify_graph_features = false;

    // Optionally set up the axes with line width, etc
    static constexpr bool setup_axes = true;

    try {
        morph::vVector<float> absc =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
        morph::vVector<float> data = absc.pow(3);
        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});

        // Here, we change the size of the graph and range of the axes (this is optional
        gv->setsize (1.33, 1);

        if constexpr (change_axes_range) {
            gv->setlimits (0,1.4,0,1.4);
        }

        if constexpr (modify_graph_features) {
            morph::DatasetStyle ds;
            ds.linewidth = 0.005;
            ds.linecolour = {1.0, 0.0, 0.0};
            ds.markerstyle = morph::markerstyle::triangle;
            ds.markersize = 0.02;
            ds.markercolour = {0.0, 0.0, 1.0};
            ds.markergap = 0.02;
            // For each dataset added there should be a set of 'datastyles' - linestyle, markerstyle, etc
            gv->setdata (absc, data, ds);
            ds.markerstyle = morph::markerstyle::square;
            ds.setcolour ({0.0, 1.0, 0.0});
            gv->setdata (absc, absc.pow(4), ds);
        } else {
            gv->policy = morph::stylepolicy::allcolour; // markers, lines, both, allcolour
            // The code here demonstrates how to include unicode characters (ss2 is "superscript 2")
            using morph::unicode;
            gv->setdata (absc, absc, "y=x");
            gv->setdata (absc, absc.pow(2)+0.05f, "y=x" + unicode::toUtf8(unicode::ss2));
            gv->setdata (absc, absc.pow(3)+0.1f, "y=x" + unicode::toUtf8(unicode::ss3));
            gv->setdata (absc, absc.pow(4)+0.15f, "y=x" + unicode::toUtf8(unicode::ss4));
            gv->setdata (absc, absc.pow(5)+0.2f, "y=x" + unicode::toUtf8(unicode::ss5));
        }

        if constexpr (setup_axes) {
            gv->axiscolour = {0.5, 0.5, 0.5};
            gv->axislinewidth = 0.01f;
            gv->axisstyle = morph::axisstyle::box;
            gv->setthickness (0.001f);
        }

        gv->finalize();

        // Add the GraphVisual (as a VisualModel*)
        v.addVisualModel (static_cast<morph::VisualModel*>(gv));

        float addn = 0.0f;
        size_t rcount = 0;
        v.render();
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            // Don't update this fast. That's crazy!
            if ((rcount++)%20 == 0) {
                gv->update (absc, absc.pow(2)*addn, 1);
                addn += 0.2f;
            }
            // want gv->update (datasets); // to update all at once. THEN I'm done.
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
