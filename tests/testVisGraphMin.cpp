/*
 * Visualize a graph. Minimal example showing how a default graph appears
 */
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vVector.h>
#include <iostream>

int main (int argc, char** argv)
{
    morph::Visual v(1024, 768, "Made with morph::GraphVisual", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);
    v.zNear = 0.001;
    v.backgroundWhite();

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        holdVis = a1.empty() ? false : true;
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});

    morph::vVector<float> absc =  {-.5, -.4, -.3, -.2, -.1, 0, .1,    .2,    .3,    .4,    .5,    .6,    .7,    .8};
    morph::vVector<float> data = absc.pow(3);
    gv->setdata (absc, data);

    gv->setup();

    v.addVisualModel (static_cast<morph::VisualModel*>(gv));

    v.render();
    if (holdVis == true) {
        while (v.readyToFinish == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }
    }

    return 0;
}
