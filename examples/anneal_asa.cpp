/*
 * Test Adaptive Simulated Annealing on a 2D objective function, visualizing the
 * progress of the algorithm.
 */

#include <morph/Anneal.h>
#include <morph/vVector.h>
#include <morph/Vector.h>
#include <morph/Config.h>
#include <morph/Hex.h>
#include <morph/HexGrid.h>
#include <morph/Visual.h>
#include <morph/VisualDataModel.h>
#include <morph/HexGridVisual.h>
#include <morph/PolygonVisual.h>
#include <morph/GraphVisual.h>
#include <iostream>
#include <string>
#include <unistd.h>

// Choose double or float for the precision used in the Anneal algorithm
typedef double F;

// A global hexgrid for the locations of the objective function
morph::HexGrid* hg = nullptr;
// And a vVector to be the data
morph::vVector<F> obj_f;

// Set up an objective function. Creates hg and populates obj_f. Note objective has
// discrete values.
void setup_objective();

// Alternative objective function
void setup_objective_boha();

// Return values of the objective function. Params contains coordinates into the
// HexGrid. Values from obj_f are returned.
F objective (const morph::vVector<F>& params);
F objective_boha (const morph::vVector<F>& params);
F objective_hg (const morph::vVector<F>& params);

int main (int argc, char** argv)
{
#ifdef USE_BOHACHEVSKY_FUNCTION
    setup_objective_boha();
#else
    setup_objective();
#endif

    // Here, our search space is 2D
    morph::vVector<F> p = { 0.45, 0.45};
    // These ranges should fall within the hexagonal domain
    morph::vVector<morph::Vector<F,2>> p_rng = {{ {-0.3, 0.3}, {-0.3, 0.3} }};

    // Set up the anneal algorithm object
    morph::Anneal<F> anneal(p, p_rng);
    // There are defaults hardcoded in Anneal.h, but these work for the cost function here:
    anneal.temperature_ratio_scale = F{1e-2};
    anneal.temperature_anneal_scale = F{200};
    anneal.cost_parameter_scale_ratio = F{3};
    anneal.acc_gen_reanneal_ratio = F{1e-6};
    anneal.delta_param = F{0.01};
    anneal.objective_repeat_precision = F{1e-6};
    anneal.f_x_best_repeat_max = 15;
    anneal.reanneal_after_steps = 100;
    anneal.exit_at_T_f = false; // If true, algo will run faster, but error will likely be non-zero
#ifndef VISUALISE
    anneal.display_temperatures = false;
    anneal.display_reanneal = false;
#endif
    // Optionally, modify ASA parameters from a JSON config specified on the command line.
    if (argc > 1) {
        morph::Config conf(argv[1]);
        if (conf.ready) {
            anneal.temperature_ratio_scale = (F)conf.getDouble ("temperature_ratio_scale", 1e-2);
            anneal.temperature_anneal_scale = (F)conf.getDouble ("temperature_anneal_scale", 200.0);
            anneal.cost_parameter_scale_ratio = (F)conf.getDouble ("cost_parameter_scale_ratio", 3.0);
            anneal.acc_gen_reanneal_ratio = (F)conf.getDouble ("acc_gen_reanneal_ratio", 1e-6);
            anneal.delta_param = (F)conf.getDouble ("delta_param", 0.01);
            anneal.objective_repeat_precision = (F)conf.getDouble ("objective_repeat_precision", 1e-6);
            anneal.f_x_best_repeat_max = conf.getUInt ("f_x_best_repeat_max", 15);
            anneal.reanneal_after_steps = conf.getUInt ("reanneal_after_steps", 100);
        } else {
            std::cerr << "Failed to open JSON config in '" << argv[1]
                      << "', continuing with default ASA parameters.\n";
        }
    }
    anneal.init();

#ifdef VISUALISE
    // Set up the visualisation
    morph::Visual v (1920, 1080, "Adaptive Simulated Annealing Example");
    v.zNear = 0.001;
    v.setSceneTransZ (-3.0f);
    v.lightingEffects (true);

    morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
    morph::HexGridVisual<F>* hgv = new morph::HexGridVisual<F>(v.shaderprog, v.tshaderprog, hg, offset);
    hgv->setScalarData (&obj_f);
#ifdef USE_BOHACHEVSKY_FUNCTION
    hgv->addLabel ("Objective: See Bohachevsky et al.", { -0.5f, -0.75f, -0.1f }, morph::colour::black);
#else
    hgv->addLabel ("Objective: 2 Gaussians and some noise", { -0.5f, -0.75f, -0.1f }, morph::colour::black);
#endif
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

    // Fourth object marks the starting place
    col = { .5f, .5f, .5f };
    polypos[2] = objective(p);
    morph::PolygonVisual* sp = new morph::PolygonVisual (v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.6f, col, 20);

    v.addVisualModel (candp);
    v.addVisualModel (bestp);
    v.addVisualModel (currp);
    v.addVisualModel (sp);

    // Add a graph to track T_i and T_cost
    morph::Vector<float> spatOff = {1.2f, -0.5f, 0.0f};
    morph::GraphVisual<F>* graph1 = new morph::GraphVisual<F> (v.shaderprog, v.tshaderprog, spatOff);
    graph1->twodimensional = true;
    graph1->setlimits (0, 1000, -10, 1);
    graph1->policy = morph::stylepolicy::lines;
    graph1->ylabel = "log(T)";
    graph1->xlabel = "Anneal time";
    graph1->prepdata ("Tparam");
    graph1->prepdata ("Tcost");
    graph1->finalize();
    v.addVisualModel (graph1);

    spatOff[0] += 1.1f;
    morph::GraphVisual<F>* graph2 = new morph::GraphVisual<F> (v.shaderprog, v.tshaderprog, spatOff);
    graph2->twodimensional = true;
    graph2->setlimits (0, 1000, -1.0f, 1.0f);
    graph2->policy = morph::stylepolicy::lines;
    graph2->ylabel = "obj value";
    graph2->xlabel = "Anneal time";
    graph2->prepdata ("f_x");
    graph2->prepdata ("f_x_best + .5");
    graph2->prepdata ("f_x_cand");
    graph2->finalize();
    v.addVisualModel (graph2);

    v.render();
#endif

    // The Optimization:
    //
    // Your job is to loop, calling anneal.step(), until anneal.state tells you to stop...
    while (anneal.state != morph::Anneal_State::ReadyToStop) {

        // ...and on each loop, compute the objectives that anneal asks you to:
        if (anneal.state == morph::Anneal_State::NeedToCompute) {
            // Compute the candidate objective value
            anneal.f_x_cand = objective (anneal.x_cand);

        } else if (anneal.state == morph::Anneal_State::NeedToComputeSet) {
            // Compute objective values for reannealing
            anneal.f_x_plusdelta = objective (anneal.x_plusdelta);
            // anneal.f_x is already computed. BUT could jump to the x_best on reanneal.

        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }

#ifdef VISUALISE
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

        // Append to the 2D graph of sums:
        graph1->append ((float)anneal.steps, std::log(anneal.T_k.mean()), 0);
        graph1->append ((float)anneal.steps, std::log(anneal.T_cost.mean()), 1);
        graph2->append ((float)anneal.steps, anneal.f_x-0.2, 0);
        graph2->append ((float)anneal.steps, anneal.f_x_best, 1);
        graph2->append ((float)anneal.steps, anneal.f_x_cand+0.2, 2);


        glfwWaitEventsTimeout (0.0166);
        v.render();
#endif
        // Finally, you need to ask the algorithm to do its stuff for one step
        anneal.step();
    }

#ifdef VISUALISE
    std::cout << "Last anneal stats: num_improved " << anneal.num_improved << ", num_worse: " << anneal.num_worse
              << ", num_worse_accepted: " << anneal.num_worse_accepted << " (as proportion: "
              << ((double)anneal.num_worse_accepted/(double)anneal.num_worse) << ")\n\n";

    std::cout << "FINISHED in " << anneal.steps << " calls to Anneal::step() (HexGrid has " << hg->num() << " hexes).\n"
              << "Best parameters: " << anneal.x_best << "\n"
              << "Best params obj: " << anneal.f_x_best
              << " vs. " << obj_f.min() << ", the true obj_f.min().\n"
              << "Final error: " <<  anneal.f_x_best - obj_f.min() << "\n";

    std::cout << "(You can close the window with 'x' or take a snapshot with 's'. 'h' for other help).\n";

    v.keepOpen();
#else
    std::cout << anneal.steps << "," << anneal.f_x_best - obj_f.min() << ","
              << anneal.f_x_best << "," << obj_f.min() << "\n";
#endif

    if (hg != nullptr) { delete hg; }
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

// Alternative objective function from Bohachevsky. This *visualises* the function, but
// during the anneal, we'll use the actual function values
void setup_objective_boha()
{
    hg = new morph::HexGrid(0.01, 2.5, 0, morph::HexDomainShape::Hexagon);
    hg->leaveAsHexagon();
    obj_f.resize (hg->num());
    F a = F{1}, b = F{2}, c=F{0.3}, d=F{0.4}, alpha=F{morph::PI_F*3.0}, gamma=F{morph::PI_F*4.0};
    for (auto h : hg->hexen) {
        obj_f[h.vi] = a*h.x*h.x + b*h.y*h.y - c * std::cos(alpha*h.x) - d * std::cos (gamma * h.y) + c + d;
    }
}

F objective (const morph::vVector<F>& params)
{
#ifdef USE_BOHACHEVSKY_FUNCTION
    return objective_boha (params);
#else
    return objective_hg (params);
#endif
}

F objective_boha (const morph::vVector<F>& params)
{
    F x = params[0];
    F y = params[1];
    F a = F{1}, b = F{2}, c=F{0.3}, d=F{0.4}, alpha=F{morph::PI_F*3.0}, gamma=F{morph::PI_F*4.0};
    F fn = a*x*x + b*y*y - c * std::cos(alpha*x) - d * std::cos (gamma * y) + c + d;
    return fn;
}

F objective_hg (const morph::vVector<F>& params)
{
    // Find the hex nearest the coordinate defined by params and return its value
    std::pair<F, F> coord;
    coord.first = params[0];
    coord.second = params[1];
    std::list<morph::Hex>::iterator hn = hg->findHexNearest (coord);
    return obj_f[hn->vi];
}
