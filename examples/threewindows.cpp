/*
 * An example showing how to create two morph::Visuals and then a third one.
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

int main()
{
    int rtn = -1;

    { // I create two morph::Visuals here in their own scope, so that I can demonstrate the creation
      // of a new, follow-on morph::Visual at the end

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
            // Visual::setContext():
            v.setContext();
            auto qvp = std::make_unique<morph::QuiverVisual<float>>(&coords, offset, &quivs, morph::ColourMapType::Cividis);
            v.bindmodel (qvp);
            qvp->finalize();
            v.addVisualModel (qvp);

            // Explicitly release context of the v Visual object, before calling setContext for the v2 object.
            v.releaseContext();

            // Set up v2 with a graph, switching to the Visual v2's context first:
            v2.setContext();
            auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
            v2.bindmodel (gv);
            morph::vvec<float> x =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
            morph::vvec<float> y = x.pow(3);
            gv->setdata (x, y);
            gv->finalize();
            v2.addVisualModel (gv);

            while (v.readyToFinish == false && v2.readyToFinish == false) {
                v.waitevents (0.018);
                v.render();
                v2.render();
            }

        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
            rtn = -1;
        }
    }

    // Both old windows have now gone out of scope. Right at the end, I re-create a morph::Visual to
    // prove that it can be done (until March 11 2024, this would fail).
    morph::Visual v(1024, 768, "This is the third (empty) window", {0.8,-0.8}, {.1,.05,.05}, 3.0f, 0.01f);
    v.showCoordArrows = true;
    v.showTitle = true;
    v.backgroundWhite();
    v.lightingEffects();

    v.keepOpen();

    return rtn;
}
