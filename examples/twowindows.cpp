/*
 * Visualize a quiver field
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/QuiverVisual.h>
#include <morph/GraphVisual.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
{
    int rtn = -1;

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows. Defines, and makes current a new
    // window and OpenGL context.
    morph::Visual v(1024, 768, "Window 1", {0.8,-0.8}, {.1,.05,.05}, 3.0f, 0.01f);
    v.showCoordArrows = true;
    v.showTitle = true;
    v.backgroundWhite();
    v.lightingEffects();

    // If I define a second Visual here, then the OpenGL context will now be 'pointing'
    // at this Visual v2
    morph::Visual v2(768, 768, "Graph on Window 2", {0.8,-0.8}, {.05,.05,.1}, 2.0f, 0.01f);
    v2.showCoordArrows = true;
    v2.showTitle = true;
    v2.backgroundWhite();
    v2.lightingEffects();

    try {
        // Set up data for the first Visual
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        std::vector<morph::vec<float, 3>> coords;
        coords.push_back ({0, 0,   0});
        coords.push_back ({1, 1,   0});
        coords.push_back ({2, 0,   0});
        coords.push_back ({1, 0.8, 0});
        coords.push_back ({2, 0.5, 0});

        std::vector<morph::vec<float, 3>> quivs;
        quivs.push_back ({0.3,   0.4,  0});
        quivs.push_back ({0.1,   0.2,  0.1});
        quivs.push_back ({-0.1,  0,    0});
        quivs.push_back ({-0.04, 0.05, -.2});
        quivs.push_back ({0.3,  -0.1,  0});

        // NB: Before adding VisualModel, and before creating a new QuiverVisual, we
        // need the OpenGL context to be correct, so set it on the first Visual, v with
        // Visual::setCurrent():
        v.setCurrent();
        unsigned int visId = v.addVisualModel (new morph::QuiverVisual<float> (v.shaderprog, &coords, offset, &quivs, morph::ColourMapType::Cividis));

        std::cout << "Added QuiverVisual with visId " << visId << std::endl;

        // Set up v2 with a graph, switching to the Visual v2's context first:
        v2.setCurrent();
        morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v2.shaderprog, v2.tshaderprog, {0,0,0});
        morph::vvec<float> x =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
        morph::vvec<float> y = x.pow(3);
        gv->setdata (x, y);
        gv->finalize();
        v2.addVisualModel (static_cast<morph::VisualModel*>(gv));

        while (v.readyToFinish == false && v2.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
            v2.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
