/*
 * Test Simulated Annealing algorithm on the Rosenbrock banana function.
 */

#include <morph/Anneal.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/TriFrameVisual.h>
#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>
#include <morph/PolygonVisual.h>
#include <morph/MathAlgo.h>
#include <iostream>
#include <chrono>

// Here's the Rosenbrock banana function
FLT banana (morph::vvec<FLT> xy) {
    FLT a = 1.0;
    FLT b = 100.0;
    FLT x = xy[0];
    FLT y = xy[1];
    FLT rtn = ((a-x)*(a-x)) + (b * (y-(x*x)) * (y-(x*x)));
   return rtn;
}

int main()
{
    // Check banana function
    FLT test = banana ({1.0, 1.0});
    std::cout << "test point on banana function = " << test << " (should be 0).\n";

    // Initial point and ranges
    morph::vvec<FLT> p = { 0.5, -0.5};
    std::cout << "Start point on banana function = " << banana(p) << ".\n";
    morph::vvec<morph::vec<FLT,2>> p_rng = {{ {-1.1, 1.1}, {-1.1, 1.1} }};

#ifdef VISUALISE
    // Set up a visual environment
    morph::Visual v(2600, 1800, "Rosenbrock bananas", {-0.8,-0.8}, {.05,.05,.05}, 2.0f, 0.01f);
    v.zNear = 0.001;
    v.zFar = 100000;
    v.fov=60;
    v.showCoordArrows = true;
    v.lightingEffects (true);

    morph::vec<float> offset = {0,0,0};
    morph::HexGrid hg (0.01, 10, 0);
    hg.setCircularBoundary (2.5);
    std::vector<FLT> banana_vals(hg.num(), 0.0f);
    for (size_t i = 0; i < hg.num(); ++i) {
        banana_vals[i] = banana ({hg.d_x[i], hg.d_y[i]});
    }
    std::pair<FLT, FLT> mm = morph::MathAlgo::maxmin(banana_vals);
    std::cout << "Banana surface max/min: " << mm.first << "," << mm.second << std::endl;
    auto hgv = std::make_unique<morph::HexGridVisual<FLT>>(v.shaderprog, v.tshaderprog, &hg, offset);
    hgv->hexVisMode = morph::HexVisMode::Triangles;
    hgv->cm.setType (morph::ColourMapType::Viridis);
    hgv->setScalarData (&banana_vals);
    hgv->zScale.setParams (0.001f, 0.0f);
    hgv->colourScale.compute_autoscale (0.01f, 5.0f);
    hgv->setAlpha (0.4f);
    hgv->finalize();
    v.addVisualModel (hgv);

    morph::vec<float, 3> polypos = { static_cast<float>(p[0]), static_cast<float>(p[1]), 0.0f };

    // One object for the 'candidate' position
    std::array<float, 3> col = { 0, 1, 0 };
    auto candup = std::make_unique<morph::PolygonVisual>(v.shaderprog, offset, polypos, morph::vec<float>({1,0,0}), 0.005f, 0.4f, col, 20);

    // A second object for the 'best' position
    col = { 1, 0, 0 };
    auto bestup = std::make_unique<morph::PolygonVisual>(v.shaderprog, offset, polypos, morph::vec<float>({1,0,0}), 0.001f, 0.8f, col, 10);

    // A third object for the currently accepted position
    col = { 1, 0, 0.7f };
    auto currup = std::make_unique<morph::PolygonVisual> (v.shaderprog, offset, polypos, morph::vec<float>({1,0,0}), 0.005f, 0.6f, col, 20);

    auto candp = v.addVisualModel (candup);
    auto bestp = v.addVisualModel (bestup);
    auto currp = v.addVisualModel (currup);
#endif

    morph::Anneal<FLT> anneal(p, p_rng);

    anneal.temperature_ratio_scale = FLT{1e-3};
    anneal.temperature_anneal_scale = FLT{200};
    anneal.cost_parameter_scale_ratio = FLT{1.5};
    anneal.acc_gen_reanneal_ratio = FLT{1e-3};
    anneal.delta_param = FLT{0.01};
    anneal.f_x_best_repeat_max = 15;
    anneal.enable_reanneal = false;
    anneal.reanneal_after_steps = 100;

    anneal.init();

    // Now do the business
    while (anneal.state != morph::Anneal_State::ReadyToStop) {

        // ...and on each loop, compute the objectives that anneal asks you to:
        if (anneal.state == morph::Anneal_State::NeedToCompute) {
            // Compute the candidate objective value
            anneal.f_x_cand = banana (anneal.x_cand);

        } else if (anneal.state == morph::Anneal_State::NeedToComputeSet) {
            // Compute objective values for reannealing
            anneal.f_x_plusdelta = banana (anneal.x_plusdelta);
            //anneal.f_x = banana (anneal.x); // no need

        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }

#ifdef VISUALISE
        // You can update the visualisation within this loop if you like:
        candp->position = { static_cast<float>(anneal.x_cand[0]),
                            static_cast<float>(anneal.x_cand[1]),
                            static_cast<float>(anneal.f_x_cand - FLT{0.15}) };
        candp->reinit();
        bestp->position = { static_cast<float>(anneal.x_best[0]),
                            static_cast<float>(anneal.x_best[1]),
                            static_cast<float>(anneal.f_x_best - FLT{0.15}) };
        bestp->reinit();
        currp->position = { static_cast<float>(anneal.x[0]),
                            static_cast<float>(anneal.x[1]),
                            static_cast<float>(anneal.f_x - FLT{0.15}) };
        currp->reinit();
        glfwWaitEventsTimeout (0.0166);
        v.render();
#endif
        anneal.step();
        //if (anneal.steps > 10) { anneal.state = morph::Anneal_State::ReadyToStop; }
    }

#ifdef VISUALISE
    std::cout << "Last anneal stats: num_improved " << anneal.num_improved << ", num_worse: " << anneal.num_worse
              << ", num_worse_accepted: " << anneal.num_worse_accepted << " (as proportion: "
              << ((double)anneal.num_worse_accepted/(double)anneal.num_worse) << ")\n\n";

    std::cout << "FINISHED in " << anneal.steps << " calls to Anneal::step().\n"
              << "Best parameters: " << anneal.x_best << "\n"
              << "Best params obj: " << anneal.f_x_best
              << " vs. [Whatever it is], the true obj_f.min().\n";

    std::cout << "(You can close the window with 'x' or take a snapshot with 's'. 'h' for other help).\n";

    v.keepOpen();
#else
    std::cout << anneal.steps << "," << anneal.f_x_best << "\n";
#endif

    return 0;
}
