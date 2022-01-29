// Twinax graph
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vVector.h>

using morph::unicode;

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Twinax GraphVisual example");
    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = new morph::GraphVisual<double> (v.shaderprog, v.tshaderprog, {0,0,0});
    // This is going to be a twin axis graph
    gv->axisstyle = morph::axisstyle::twinax;
    // Data for the x axis. A vVector is like std::vector, but with built-in maths methods
    morph::vVector<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);

    // Set a graph up of x^3
    std::string ds1legend = unicode::toUtf8 (unicode::alpha);
    ds1legend += "(x) = x" + unicode::toUtf8 (unicode::ss3);
    gv->setdata (x, x.pow(3), ds1legend);
    gv->ylabel = unicode::toUtf8 (unicode::alpha);

    // And 100x^2
    std::string ds2legend = unicode::toUtf8 (unicode::beta);
    ds2legend += "(x) = 100x" + unicode::toUtf8 (unicode::ss2);
    gv->setdata_rightaxis (x, x.pow(2)*100, ds2legend);
    gv->ylabel2 = unicode::toUtf8 (unicode::beta);

    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'x'
    v.keepOpen();
    // When v goes out of scope, gv will be deallocated
    return 0;
}
