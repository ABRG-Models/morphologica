/*
 * Test simulated annealing using Bohachevsy, Johnson and Stein's function
 */

#include <morph/Anneal.h>
#include <morph/vVector.h>
#include <morph/Vector.h>
#include <morph/Hex.h>
#include <morph/HexGrid.h>
#include <morph/MathConst.h>
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

// This sets up the function ax^2 +by^2 -c cos(alpha x) -d cos (gamma y) + c + d. This
// _could_ be used just for vis, but I will use its discrete values as the values of the
// objective, too.
void setup_objective()
{
    hg = new morph::HexGrid(0.01, 2.5, 0, morph::HexDomainShape::Hexagon);
    obj_f.resize (hg->num());
    float a = 1.0f, b = 2.0f, c=0.3f, d=0.4f, alpha=morph::PI_F*3.0f, gamma=morph::PI_F*4.0f;
    for (auto h : hg->hexen) {
        obj_f[h.vi] = a*h.x*h.x + b*h.y*h.y - c * std::cos(alpha*h.x) - d * std::cos (gamma * h.y) + c + d;
    }
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

    // Visualise the objective function with a HexGridVisual
    morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };
    morph::HexGridVisual<float>* hgv = new morph::HexGridVisual<float>(v.shaderprog, v.tshaderprog, hg, offset);
    hgv->setScalarData (&obj_f);
    hgv->addLabel ("Bohachevsky et al Objective", { -0.3f, -0.45f, 0.01f }, morph::colour::black);
    hgv->finalize();
    v.addVisualModel (hgv);

    // Here, our search space is 2D; set up the initial parameters of the objective and the permitted ranges of exploration
    morph::vVector<float> p = { 1.0f, 1.0f};
    // These ranges should fall within the hexagonal domain of the HexGrid
    morph::vVector<morph::Vector<float,2>> p_rng = {{ {-1.05f, 1.05f}, {-1.05f, 1.05f} }};

    // A green line for the 'candidate' position
    morph::Vector<float, 3> init_line_pos = { p[0], p[1], 0.0f };
    std::array<float, 3> col = { 0, 1, 0 };
    morph::PolygonVisual* candp = new morph::PolygonVisual(v.shaderprog, offset, init_line_pos, {1,0,0}, 0.005f, 0.4f, col, 20);
    v.addVisualModel (candp);

    // A red line for the 'best' position
    col = { 1, 0, 0 };
    morph::PolygonVisual* bestp = new morph::PolygonVisual(v.shaderprog, offset, init_line_pos, {1,0,0}, 0.005f, 0.6f, col, 20);
    v.addVisualModel (bestp);

    // A pink line for the current position
    col = { 1, 0, 0.7f };
    morph::PolygonVisual* currp = new morph::PolygonVisual (v.shaderprog, offset, init_line_pos, {1,0,0}, 0.005f, 0.6f, col, 20);
    v.addVisualModel (currp);

    v.render();

    // Set up the morph::Anneal object
    morph::Anneal<float> anneal(p, p_rng);
    anneal.num_operations = 150;
    anneal.range_mult = 0.15;

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

    delete hg;
    return rtn;
}
