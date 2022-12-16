// This is a graph which updates on each step. To test for a bug, but also to show how
// a graph can be completely redrawn each time, if required.
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>

int main()
{
    morph::Visual v(1024, 768, "Continuous redrawing of GraphVisual");

    auto gv = std::make_unique<morph::GraphVisual<double>> (v.shaderprog, v.tshaderprog, morph::vec<float>({0,0,0}));

    morph::vvec<double> x;
    x.linspace (-morph::mathconst<double>::pi, morph::mathconst<double>::pi, 100);

    double dx = 0.0;

    gv->setdata (x, (x+dx).sin());
    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish == false) {
        dx += 0.01;
        glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz
        gvp->update (x, (x+dx).sin(), 0);
        v.render();
    }

    return 0;
}
