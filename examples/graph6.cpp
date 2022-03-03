// This is a graph which updates on each step. To test for a bug, but also to show how
// a graph can be completely redrawn each time, if required.
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vVector.h>
#include <morph/MathConst.h>

int main()
{
    morph::Visual v(1024, 768, "Continuous redrawing of GraphVisual");

    auto gv = new morph::GraphVisual<double> (v.shaderprog, v.tshaderprog, {0,0,0});

    morph::vVector<double> x;
    x.linspace (-morph::mathconst<double>::pi, morph::mathconst<double>::pi, 100);

    double dx = 0.0;

    gv->setdata (x, (x+dx).sin());
    gv->finalize();

    v.addVisualModel (gv);

    while (v.readyToFinish == false) {
        dx += 0.01;
        glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
        gv->update (x, (x+dx).sin(), 0);
        v.render();
    }

    return 0;
}
