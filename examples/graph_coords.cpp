// This version uses containers of coordinates to set the graph data.

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>
#include <morph/Random.h>

int main()
{
    morph::Visual v(1024, 768, "Coordinates in GraphVisual");
    v.setSceneTrans (morph::vec<float,3>({-0.458656f, -0.428112f, -2.5f}));

    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);

    // Choose marker policy for this graph
    gv->policy = morph::stylepolicy::markers;
    // And set the graph limits suitably
    gv->setlimits (0, 1, 0, 1);

    // Two random number generators
    morph::RandNormal<double> rn1 (0.2, 0.07);
    morph::RandNormal<double> rn2 (0.6, 0.04);

    // Three coordinates, which will be randomly sampled using rngs
    morph::vec<double, 2> x1 = {rn1.get(), rn1.get()};
    morph::vec<double, 2> x2 = {rn2.get(), rn1.get()};
    morph::vec<double, 2> x3 = {rn2.get(), rn2.get()};

    morph::vvec<morph::vec<double, 2>> coords = { x1, x2, x3 };

    gv->setdata (coords);
    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish() == false) {
        glfwWaitEventsTimeout (0.01667); // 16.67 ms ~ 60 Hz

        // Re-draw numbers for the coordinates
        x1 = {rn1.get(), rn1.get()};
        x2 = {rn2.get(), rn1.get()};
        x3 = {rn2.get(), rn2.get()};
        coords = { x1, x2, x3 };

        // Update the graph
        gvp->update (coords, 0);
        v.render();
    }

    return 0;
}
