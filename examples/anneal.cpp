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

// A global hexgrid for the locations of the objective function
morph::HexGrid* hg;
// And a vVector to be the data
morph::vVector<float> obj_f;

// This sets up a noisy 2D objective function with multiple peaks
void setup_objective()
{
    hg = new morph::HexGrid(0.01, 1.5, 0, morph::HexDomainShape::Hexagon);
    obj_f.resize (hg->num());

    // Create 2 Gaussians and sum them as the main features
    morph::vVector<float> obj_f_a(hg->num(), float{0});
    morph::vVector<float> obj_f_b(hg->num(), float{0});

    // Now assign an analytical function to the thing - make it a couple of Gaussians
    float sigma = 0.045f;
    float one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
    float two_sigma_sq = 2.0f * sigma * sigma;
    float gauss = 0;
    float sum = 0;
    morph::Hex chex = *hg->vhexen[200];
    morph::Hex chex2 = *hg->vhexen[2000];
    for (auto& k : hg->hexen) {
        // Gaussian profile based on the hex's distance from centre, which is
        // already computed in each Hex as Hex::r. Don't want this for these. Want dist from some hex/coords
        float r = k.distanceFrom (chex);
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(r*r) / two_sigma_sq ));
        obj_f_a[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : hg->hexen) { obj_f_a[k.vi] *= float{0.01}; }

    sigma = 0.1f;
    one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
    two_sigma_sq = 2.0f * sigma * sigma;
    gauss = 0;
    sum = 0;
    for (auto& k : hg->hexen) {
        float r = k.distanceFrom (chex2);
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(r*r) / two_sigma_sq ));
        obj_f_b[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : hg->hexen) { obj_f_b[k.vi] *= float{0.01}; }

    // Make noise
    morph::vVector<float> noise(hg->num());
    noise.randomize();
    noise *= float{0.2};

    // Then add em up
    obj_f = obj_f_a + obj_f_b + noise;

    // Then smooth...
    // Create a circular HexGrid to contain the Gaussian convolution kernel
    sigma = 0.005f;
    one_over_sigma_root_2_pi = 1 / sigma * 2.506628275;
    two_sigma_sq = 2.0f * sigma * sigma;
    morph::HexGrid kernel(0.01, 20.0f*sigma, 0, morph::HexDomainShape::Boundary);
    kernel.setCircularBoundary (6.0f*sigma);
    std::vector<float> kerneldata (kernel.num(), 0.0f);
    gauss = 0;
    sum = 0;
    for (auto& k : kernel.hexen) {
        gauss = (one_over_sigma_root_2_pi * std::exp ( -(k.r*k.r) / two_sigma_sq ));
        kerneldata[k.vi] = gauss;
        sum += gauss;
    }
    for (auto& k : kernel.hexen) { kerneldata[k.vi] /= sum; }

    // A vector for the result
    morph::vVector<float> convolved (hg->num(), float{0});

    // Call the convolution method from HexGrid:
    hg->convolve (kernel, kerneldata, obj_f, convolved);

    obj_f.swap (convolved);

    // And finally, invert (so we go downhill to the valleys)
    obj_f = -obj_f;
}

float objective (const morph::vVector<float>& params)
{
    // Find the hex nearest the coordinate defined by params and return its value
    std::pair<float, float> coord;
    coord.first = params[0];
    coord.second = params[1];
    std::list<morph::Hex>::iterator hn = hg->findHexNearest (coord);
    return obj_f[hn->vi];
}

int main()
{
    setup_objective();

    morph::Visual v(1920,1080,"Simulated Annealing Example");
    v.zNear = 0.001;
    v.setSceneTransZ (-3.0f);
    v.lightingEffects (true);

    morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
    morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, hg, offset);
    hgv->setScalarData (&obj_f);
    hgv->addLabel ("Objective", { -0.3f, -0.45f, 0.01f }, morph::colour::black);
    hgv->finalize();
    v.addVisualModel (hgv);

    // Here, our search space is 2D
    morph::vVector<float> p = { 0.45, 0.45};
    // These ranges should fall within the hexagonal domain
    morph::vVector<morph::Vector<float,2>> p_rng = {{ {-0.3, 0.3}, {-0.3, 0.3} }};

    morph::Vector<float, 3> polypos = { p[0], p[1], 0.0f };

    // One object for the 'candidate' position
    std::array<float, 3> col = { 0, 1, 0 };
    morph::PolygonVisual polyc (v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.4f, col, 20);
    morph::PolygonVisual* candp = &polyc;

    // A second object for the 'best' position
    col = { 1, 0, 0 };
    morph::PolygonVisual polyb (v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.6f, col, 20);
    morph::PolygonVisual* bestp = &polyb;

    // A third object for the currently accepted position
    col = { 1, 0, 0.7f };
    morph::PolygonVisual polycur (v.shaderprog, offset, polypos, {1,0,0}, 0.005f, 0.6f, col, 20);
    morph::PolygonVisual* currp = &polycur;

    v.addVisualModel (candp);
    v.addVisualModel (bestp);
    v.addVisualModel (currp);
    v.render();

    // Set up the morph::Anneal object
    morph::Anneal<float> anneal(p, p_rng);
    anneal.num_operations = 2000;
    anneal.range_mult = 0.1;

    // Now do the business
    while (anneal.state != morph::Anneal_State::ReadyToStop) {
        if (anneal.state == morph::Anneal_State::NeedToCompute) {
            // Take the candidate parameters from the Anneal object and compute the candidate objective value
            anneal.set_f_x_cand (objective (anneal.x_cand));
            // Update the visualisation
            candp->position = { anneal.x_cand[0], anneal.x_cand[1], anneal.f_x_cand - 0.15f };
            candp->reinit();
            bestp->position = { anneal.x_best[0], anneal.x_best[1], anneal.f_x_best - 0.15f };
            bestp->reinit();
            currp->position = { anneal.x[0], anneal.x[1], anneal.f_x - 0.15f };
            currp->reinit();
        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }

        glfwWaitEventsTimeout (0.05); // 0.01667 16.67 ms ~ 60 Hz
        v.render();

        // A step of the Anneal algoirhtm involves reducing the temperature and stochastically selecting new candidate parameters
        anneal.step();
    }

    std::cout << "FINISHED! Best approximation: (Params: " << anneal.x_best << ") has value "
              << anneal.f_x_best << " compare with obj_f.min(): " << obj_f.min() << std::endl;
    std::cout << "Anneal stats: num_improved " << anneal.num_improved << ", num_worse: " << anneal.num_worse
              << ", num_worse_accepted: " << anneal.num_worse_accepted << " (as proportion: "
              << ((double)anneal.num_worse_accepted/(double)anneal.num_worse) << ")" << std::endl;

    v.keepOpen();

    int rtn = -1;
    // Add success test

    return rtn;
}
