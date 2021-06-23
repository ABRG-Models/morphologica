/*
 * The scalar products of a set of random vectors should follow the beta distribution.
 */

#include <morph/Vector.h>
#include <morph/vVector.h>
#include <morph/Random.h>
#include <morph/histo.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <iostream>

int main()
{
    static constexpr size_t N = 1000000;
    static constexpr size_t n = 2;

    // Create N normalized vectors at random.
    morph::vVector<morph::Vector<float, n>> vVecs(N);
    morph::RandUniform<float> rng(-1.0f, 1.0f);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < n; ++j) {
            vVecs[i][j] = rng.get();
        }
        vVecs[i].renormalize();
    }

    // Get scalar products between pairs
    morph::vVector<float> sp (N/n);
    for (size_t i = 0; i < N/n; ++i) { sp[i] = vVecs[i].dot (vVecs[i+N/n]); } // No good for n!=2

    // Make a histogram of the scalar product pairs
    morph::histo h(sp, 100);

    // Set up a morph::Visual for a graph
    morph::Visual v(1024, 768, "Histogram", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);

    // Create a new GraphVisual with offset within the scene of 0,0,0
    morph::GraphVisual<float>* gv = new morph::GraphVisual<float> (v.shaderprog, v.tshaderprog, {0,0,0});
    gv->policy = morph::stylepolicy::bar;
    gv->twodimensional = false;
    gv->setdata (h);
    gv->xlabel = "Scalar product";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (static_cast<morph::VisualModel*>(gv));

    // Render the graph until user exits
    v.render();
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
