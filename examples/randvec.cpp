/*
 * The scalar products of a set of randomly directed, normalised vectors should follow
 * the beta distribution. Here, I multiply their length by a normally distributed amount
 * near 1.
 */

#include <morph/vec.h>
#include <morph/vvec.h>
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
    morph::vvec<morph::vec<float, n>> vVecs(N);
    morph::RandUniform<float> rn_u(-1.0f, 1.0f);
    morph::RandNormal<float> rn_n(1.0f, 0.06f);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < n; ++j) {
            vVecs[i][j] = rn_u.get();
        }
        // Renormalise
        vVecs[i].renormalize();
        // Multiply length by a normally distributed amount near 1
        vVecs[i] *= rn_n.get();
    }

    // Get scalar products between pairs
    morph::vvec<float> sp (N/n);
    for (size_t i = 0; i < N/n; ++i) { sp[i] = vVecs[i].dot (vVecs[i+N/n]); } // No good for n!=2

    // Make a histogram of the scalar product pairs
    morph::histo h(sp, 100);

    // Set up a morph::Visual for a graph
    morph::Visual v(1024, 768, "Histogram", {-0.8,-0.8}, {.1,.1,.1}, 1.0f, 0.01f);

    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<float>> (v.shaderprog, v.tshaderprog, morph::vec<float>({0,0,0}));
    gv->setdata (h); // to become gv->add_bargraph (h [,morph::colour::darkorchid1] [,morph::colour::orchid2])
    gv->xlabel = "Scalar product";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.render();
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }

    return 0;
}
