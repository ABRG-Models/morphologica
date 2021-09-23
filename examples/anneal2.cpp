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

void setup_objective()
{
    hg = new morph::HexGrid(0.01, 2.5, 0, morph::HexDomainShape::Hexagon);
    obj_f.resize (hg->num());
    F a = F{1}, b = F{2}, c=F{0.3}, d=F{0.4}, alpha=F{morph::PI_F*3.0}, gamma=F{morph::PI_F*4.0};
    for (auto h : hg->hexen) {
        obj_f[h.vi] = a*h.x*h.x + b*h.y*h.y - c * std::cos(alpha*h.x) - d * std::cos (gamma * h.y) + c + d;
    }
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
    hgv->addLabel ("Objective: As in Bohachevsky et al", { -0.5f, -0.75f, -0.1f }, morph::colour::black);
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

    // Now do the business
    while (anneal.state != morph::Anneal_State::ReadyToStop) {

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

        // Update the visualisation
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
        // Render the graphics - timeout can slow down the example
        glfwWaitEventsTimeout (0.05); // 0.01667 16.67 ms ~ 60 Hz
        v.render();

        // Ask the algorithm to do its stuff for one step
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
