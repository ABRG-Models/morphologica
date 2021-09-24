/*
 * Test Adaptive Simulated Annealing on a 2D objective function. Just stats, no vis.
 */

#include <morph/Anneal.h>
#include <morph/vVector.h>
#include <morph/Vector.h>
#include <morph/Config.h>
#include <morph/Hex.h>
#include <morph/HexGrid.h>
#include <iostream>
#include <string>
#include <unistd.h>

// Choose double or float for the precision used in the Anneal algorithm
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

int main (int argc, char** argv)
{
    // This allocates the HexGrid, hg
    setup_objective();

    // Here, our search space is 2D
    morph::vVector<F> p = { 0.45, 0.45};
    // These ranges should fall within the hexagonal domain
    morph::vVector<morph::Vector<F,2>> p_rng = {{ {-0.3, 0.3}, {-0.3, 0.3} }};

    // Set up the anneal algorithm object
    morph::Anneal<F> anneal(p, p_rng);
    // Defaults are hardcoded in Anneal.h, but can be changed here:
    anneal.temperature_ratio_scale = F{1e-5}; // 1e-5 is default
    anneal.temperature_anneal_scale = F{100}; // 100 default
    anneal.cost_parameter_scale_ratio = F{1}; // 1 is default
    anneal.acc_gen_reanneal_ratio = F{0.7};   // There may well be a better default
    anneal.partials_samples = 5;
    anneal.f_x_best_repeat_max = 10;
    anneal.reanneal_after_steps = 100;
    // Optionally, modify ASA parameters from a JSON config specified on the command line.
    if (argc > 1) {
        morph::Config conf(argv[1]);
        if (conf.ready) {
            anneal.temperature_ratio_scale = (F)conf.getDouble ("temperature_ratio_scale", 1e-5);
            anneal.temperature_anneal_scale = (F)conf.getDouble ("temperature_anneal_scale", 100.0);
            anneal.cost_parameter_scale_ratio = (F)conf.getDouble ("cost_parameter_scale_ratio", 1.0);
            anneal.acc_gen_reanneal_ratio = (F)conf.getDouble ("acc_gen_reanneal_ratio", 0.7);
            anneal.partials_samples = conf.getUInt ("partials_samples", 5);
            anneal.f_x_best_repeat_max = conf.getUInt ("f_x_best_repeat_max", 10);
            anneal.reanneal_after_steps = conf.getUInt ("reanneal_after_steps", 100);
        } else {
            std::cerr << "Failed to open JSON config in '" << argv[1]
                      << "', continuing with default ASA parameters.\n";
        }
    }
    anneal.init();

    // The Optimization:
    //
    // Your job is to loop, calling anneal.step(), until anneal.state tells you to stop...
    while (anneal.state != morph::Anneal_State::ReadyToStop) {

        // ...and on each loop, compute the objectives that anneal asks you to:
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

        // Finally, you need to ask the algorithm to do its stuff for one step
        anneal.step();
    }

    F final_error = anneal.f_x_best - obj_f.min();

    std::cout << anneal.steps << "," << final_error << "," << anneal.f_x_best << "," << obj_f.min() << "\n";

    //std::cout << "FINISHED in " << anneal.steps << " calls to Anneal::step().\n"
    //          << "Best parameters: " << anneal.x_best << "\n"
    //          << "Best params obj: " << anneal.f_x_best
    //          << " vs. " << obj_f.min() << ", the true obj_f.min().\n"
    //          << "Final error: " <<  final_error << "\n";

    delete hg;
    return final_error == F{0} ? 0 : 1;
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
