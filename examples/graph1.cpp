/*
 * Visualize a graph. Minimal example showing how a default graph appears
 */
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vVector.h>
#include <iostream>

int main (int argc, char** argv)
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Made with morph::GraphVisual", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);
    v.backgroundWhite();

    // Create a new GraphVisual with offset within the scene of 0,0,0
    morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});

    // Create some data (y = x^3):
    morph::vVector<float> x =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
    morph::vVector<float> y = x.pow(3);

    // Set the data into the graph
    gv->setdata (x, y);

    // Complete the setup
    gv->finalize();

    // Add the GraphVisual to the Visual scene
    v.addVisualModel (static_cast<morph::VisualModel*>(gv));

    // Render the scene on the screen
    v.render();
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
