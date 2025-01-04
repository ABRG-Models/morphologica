// A line graph showing how line segments work nicely

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Made with morph::GraphVisual");
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
    // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
    v.bindmodel (gv);
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (0, 10, 11);
    // Hand chosen numbers
    morph::vvec<double> y = { 5, 8, 2, 9, 1, 2, 4, 5, 8, 0, 1 };
    // Choose a line graph by crezating a lines stylepolicy datasetstyle.
    morph::DatasetStyle ds (morph::stylepolicy::lines);
    ds.linecolour = morph::colour::crimson;
    // Now set the data
    gv->setdata (x, y, ds);
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
    // Because v owns the unique_ptr to the GraphVisual, its memory will be deallocated when v goes out of scope.
    return 0;
}
