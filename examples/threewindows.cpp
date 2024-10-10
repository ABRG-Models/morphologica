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

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows. Defines, and makes current a new
    // window and OpenGL context.
    morph::Visual v(1024, 768, "Close Window 1 and Window 3 briefly appears then program exits", {0.8,-0.8}, {.1,.05,.05}, 3.0f, 0.01f);
    v.showCoordArrows = true;
    v.showTitle = true;
    v.backgroundWhite();
    v.lightingEffects();

    { // I create two morph::Visuals here in their own scope, so that I can demonstrate the creation
      // of a new, follow-on morph::Visual at the end

        // v2 is a second window and OpenGL context
        morph::Visual v2(768, 768, "Close Window 2 and Window 3 appears", {0.8,-0.8}, {.05,.05,.1}, 2.0f, 0.01f);
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

            auto qvp = std::make_unique<morph::QuiverVisual<float>>(&coords, offset, &quivs, morph::ColourMapType::Cividis);
            v.bindmodel (qvp);
            qvp->finalize();
            v.addVisualModel (qvp);

            // Set up v2 with a graph
            auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
            v2.bindmodel (gv);
            morph::vvec<float> x =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
            morph::vvec<float> y = x.pow(3);
            gv->setdata (x, y);
            gv->finalize();
            v2.addVisualModel (gv);

            while (v.readyToFinish == false && v2.readyToFinish == false) {
                v.waitevents (0.018);
                v2.render();
                v.render();
            }
            v.render();

        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
            rtn = -1;
        }
    }

    v.render();

    // Both old windows have now gone out of scope. Right at the end, I re-create a morph::Visual to
    // prove that it can be done (until March 11 2024, this would fail).
    morph::Visual v3(1024, 768, "This is the third (empty) window", {0.8,-0.8}, {.1,.05,.05}, 3.0f, 0.01f);
    v3.showCoordArrows = true;
    v3.showTitle = true;
    v3.backgroundWhite();
    v3.lightingEffects();

    while (v3.readyToFinish == false && v.readyToFinish == false) {
        v3.waitevents (0.018);
        v.render();
        v3.render();
    }

    return rtn;
}
