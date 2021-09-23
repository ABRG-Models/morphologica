/*
 * Test simulated annealing
 */

#include <morph/Anneal.h>
#include <morph/vVector.h>
#include <morph/Vector.h>
#include <morph/Hex.h>
#include <morph/HexGrid.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/PolygonVisual.h>
#include <iostream>
#include <unistd.h>

// double or float
typedef double F;

// A global hexgrid for the locations of the objective function
morph::HexGrid* hg;
// And a vVector to be the data
morph::vVector<F> obj_f;

// Set up an objective function. Creates hg and populates obj_f. Note objective has
// discrete values.
void setup_objective();

// Return values of the objective function. Params contains coordinates into the
// HexGrid. Values from obj_f are returned.
F objective (const morph::vVector<F>& params);

int main()
{
    // This allocates the HexGrid, hg
    setup_objective();

    // Here, our search space is 2D
    morph::vVector<F> p = { 0.45, 0.45};
    // These ranges should fall within the hexagonal domain
    morph::vVector<morph::Vector<F,2>> p_rng = {{ {-0.3, 0.3}, {-0.3, 0.3} }};

    // Set up the anneal algorithm object
    morph::Anneal<F> anneal(p, p_rng);
    anneal.temperature_ratio_scale = F{1e-5}; // 1e-5 is default
    anneal.temperature_anneal_scale = F{100}; // 100 default
    anneal.cost_parameter_scale_ratio = F{1}; // 1 is default
    anneal.acc_gen_reanneal_ratio = F{0.7};   // Don't know a good default
    anneal.partials_samples = 5;
    anneal.init();

    // Set up the visualisation
    morph::Visual v (1920, 1080, "Simulated Annealing Example");
    v.zNear = 0.001;
    v.setSceneTransZ (-3.0f);
    v.lightingEffects (true);

    morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
    morph::HexGridVisual<F>* hgv = new morph::HexGridVisual<F>(v.shaderprog, v.tshaderprog, hg, offset);
    hgv->setScalarData (&obj_f);
    hgv->addLabel ("Objective: 2 Gaussians and some noise", { -0.5f, -0.75f, -0.1f }, morph::colour::black);
    hgv->finalize();
    v.addVisualModel (hgv);

    morph::Vector<float, 3> polypos = { static_cast<float>(p[0]), static_cast<float>(p[1]), 0.0f };

    // One object for the 'candidate' position
    std::array<float, 3> col = { 0, 1, 0 };
    morph::PolygonVisual* candp = new morph::PolygonVisual(v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.4f, col, 20);

    // A second object for the 'best' position
    col = { 1, 0, 0 };
    morph::PolygonVisual* bestp = new morph::PolygonVisual(v.shaderprog, offset, polypos, {1,0,0}, 0.001f, 0.8f, col, 10);

    // A third object for the currently accepted position
    col = { 1, 0, 0.7f };
    morph::PolygonVisual* currp = new morph::PolygonVisual (v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.6f, col, 20);

    v.addVisualModel (candp);
    v.addVisualModel (bestp);
    v.addVisualModel (currp);
    v.render();

    // The Optimization:
    //
    // Your job is to loop, calling anneal.step() until anneal.state tells you to stop...
    while (anneal.state != morph::Anneal_State::ReadyToStop) {

        // ...and on each loop, compute the objectives that anneal asks to to:
        if (anneal.state == morph::Anneal_State::NeedToCompute) {
            // Compute the candidate objective value
            anneal.f_x_cand = objective (anneal.x_cand);

        } else if (anneal.state == morph::Anneal_State::NeedToComputeSet) {
            // Compute a set of objective values for reannealing
            for (unsigned int i = 0; i < anneal.partials_samples; ++i) {
                anneal.f_x_set[i] = objective (anneal.x_set[i]);
            }

        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }

        // You can update the visualisation within this loop if you like:
        candp->position = { static_cast<float>(anneal.x_cand[0]),
                            static_cast<float>(anneal.x_cand[1]),
                            static_cast<float>(anneal.f_x_cand - F{0.15}) };
        candp->reinit();
        bestp->position = { static_cast<float>(anneal.x_best[0]),
                            static_cast<float>(anneal.x_best[1]),
                            static_cast<float>(anneal.f_x_best - F{0.15}) };
        bestp->reinit();
        currp->position = { static_cast<float>(anneal.x[0]),
                            static_cast<float>(anneal.x[1]),
                            static_cast<float>(anneal.f_x - F{0.15}) };
        currp->reinit();
        glfwWaitEventsTimeout (0.0166);
        v.render();

        // Finally, you need to ask the algorithm to do its stuff for one step
        anneal.step();
    }

    std::cout << "FINISHED in " << anneal.steps << " steps. Best approximation: (Params: " << anneal.x_best << ") has value "
              << anneal.f_x_best << " compare with obj_f.min(): " << obj_f.min() << std::endl;
    std::cout << "Anneal stats: num_improved " << anneal.num_improved << ", num_worse: " << anneal.num_worse
              << ", num_worse_accepted: " << anneal.num_worse_accepted << " (as proportion: "
              << ((double)anneal.num_worse_accepted/(double)anneal.num_worse) << ")" << std::endl;

    v.keepOpen();

    delete hg;
    return 0;
}

// This sets up a noisy 2D objective function with multiple peaks
void setup_objective()
{
    hg = new morph::HexGrid(0.01, 1.5, 0, morph::HexDomainShape::Hexagon);
    hg->leaveAsHexagon();
    obj_f.resize (hg->num());

    // Create 2 Gaussians and sum them as the main features
    morph::vVector<F> obj_f_a(hg->num(), F{0});
    morph::vVector<F> obj_f_b(hg->num(), F{0});

    // Now assign an analytical function to the thing - make it a couple of Gaussians
    F sigma = F{0.045};
    F one_over_sigma_root_2_pi = F{1} / sigma * F{2.506628275};
    F two_sigma_sq = F{2} * sigma * sigma;
    F gauss = F{0};
    F sum = F{0};
    morph::Hex chex = *hg->vhexen[200];
    morph::Hex chex2 = *hg->vhexen[2000];
    for (auto& k : hg->hexen) {
        // Gaussian profile based on the hex's distance from centre, which is
        // already computed in each Hex as Hex::r. Don't want this for these. Want dist from some hex/coords
        F r = k.distanceFrom (chex);
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(r*r) / two_sigma_sq ));
        obj_f_a[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : hg->hexen) { obj_f_a[k.vi] *= F{0.01}; }

    sigma = F{0.1};
    one_over_sigma_root_2_pi = F{1} / sigma * F{2.506628275};
    two_sigma_sq = F{2} * sigma * sigma;
    gauss =  F{0};
    sum =  F{0};
    for (auto& k : hg->hexen) {
        F r = k.distanceFrom (chex2);
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(r*r) / two_sigma_sq ));
        obj_f_b[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : hg->hexen) { obj_f_b[k.vi] *= F{0.01}; }

    // Make noise
    morph::vVector<F> noise(hg->num());
    noise.randomize();
    noise *= F{0.2};

    // Then add em up
    obj_f = obj_f_a + obj_f_b + noise;

    // Then smooth...
    // Create a circular HexGrid to contain the Gaussian convolution kernel
    sigma = F{0.005};
    one_over_sigma_root_2_pi = F{1} / sigma * F{2.506628275};
    two_sigma_sq = F{2} * sigma * sigma;
    morph::HexGrid kernel(F{0.01}, F{20}*sigma, 0, morph::HexDomainShape::Boundary);
    kernel.setCircularBoundary (F{6}*sigma);
    std::vector<F> kerneldata (kernel.num(), F{0});
    gauss = F{0};
    sum = F{0};
    for (auto& k : kernel.hexen) {
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(k.r*k.r) / two_sigma_sq ));
        kerneldata[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : kernel.hexen) { kerneldata[k.vi] /= sum; }

    // A vector for the result
    morph::vVector<F> convolved (hg->num(), F{0});

    // Call the convolution method from HexGrid:
    hg->convolve (kernel, kerneldata, obj_f, convolved);

    obj_f.swap (convolved);

    // And finally, invert (so we go downhill to the valleys)
    obj_f = -obj_f;
}

F objective (const morph::vVector<F>& params)
{
    // Find the hex nearest the coordinate defined by params and return its value
    std::pair<F, F> coord;
    coord.first = params[0];
    coord.second = params[1];
    std::list<morph::Hex>::iterator hn = hg->findHexNearest (coord);
    return obj_f[hn->vi];
}
