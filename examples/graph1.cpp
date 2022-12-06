// Visualize a graph. Minimal example showing how a default graph appears
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Made with morph::GraphVisual");
    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = new morph::GraphVisual<double> (v.shaderprog, v.tshaderprog, {0,0,0});
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);
    // Set a graph up of y = x^3
    gv->setdata (x, x.pow(3));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'x'
    v.keepOpen();
    // When v goes out of scope, gv will be deallocated
    return 0;
}
