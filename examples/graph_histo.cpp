/*
 * Histogram example
 */

#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/histo.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <iostream>

int main()
{
    using F = float;
    // Find the distribution of the values of 1000 * sin(x) for 0 <= x <= 2pi (in 1000 steps)
    morph::vvec<F> numbers (1000);
    numbers.linspace (0.0f, morph::mathconst<F>::two_pi);
    for (auto& num : numbers) { num = 1000.0f * std::sin (num); }

    // Make a histogram of the values of sin(x) with 30 bins
    morph::histo h(numbers, 54);

    // Set up a morph::Visual for a graph
    morph::Visual v(1024, 768, "Histogram");
    v.setSceneTrans (morph::vec<float,3>({-0.539211f, -0.401911f, -2.8f}));

    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<F>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);
    gv->setdata (h); // to become gv->add_bargraph (h [,morph::colour::darkorchid1] [,morph::colour::orchid2])
    gv->xlabel = "1000 sin(x)";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.keepOpen();

    return 0;
}
