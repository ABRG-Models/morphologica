/*
 * Reimplementation of retinotectal model presented by Hugh Simpson and Geoffrey
 * Goodhill in "A simple model can unify a broad range of phenomena in retinotectal map
 * development", Biol Cybern (2011) 104:9-29
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include <morph/CartGrid.h>
#include <morph/Config.h>
#include <morph/Random.h>
#include <morph/Visual.h>
#include <morph/CartGridVisual.h>

#include "branchvisual.h"
#include "branch.h"

template<typename T>
struct SimpsonGoodhill
{
    SimpsonGoodhill (morph::Config* cfg)
    {
        this->conf = cfg;
        this->init();
    }
    ~SimpsonGoodhill() { delete this->retina; }

    void run()
    {
        for (unsigned int i = 0; i < this->conf->getUInt ("steps", 1000); ++i) {
            this->step();
            if (i%100 == 0) {
                this->vis();
                std::cout << "step " << i << "\n";
            }
        }
        std::cout << "Done simulating\n";
        this->v->keepOpen();
    }

    void vis()
    {
        glfwPollEvents(); //glfwWaitEventsTimeout (0.1); // to add artificial slowing
        this->bv->reinit();
        this->v->render();
    }

    void step()
    {
        // Compute the next position for each branch:
#pragma omp parallel for
        for (auto& b : this->branches) { b.compute_next (this->branches, this->m); }
        // Once 'next' has been updated, add next to path:
        for (auto& b : this->branches) { b.path.push_back (b.next); }
    }

    void init()
    {
        // Simulation init
        morph::RandUniform<T, std::mt19937> rng;
        // gr is grid element length
        T gr = T{1}/T{rgcside};
        std::cout << "Grid element length " << gr << std::endl;
        this->retina = new morph::CartGrid(gr, gr, 1, 1);
        this->retina->setBoundaryOnOuterEdge();
        std::cout << "Retina has " << this->retina->num() << " cells\n";
        this->branches.resize(this->retina->num() * bpa);
        std::vector<T> rn = rng.get (this->retina->num() * 2 * bpa);
        for (unsigned int i = 0; i < this->branches.size(); ++i) {
            // Reserve sufficient memory space in the vector
            this->branches[i].path.reserve (this->conf->getUInt ("steps", 1000));
            // Set the branch's termination zone
            unsigned int ri = i/bpa; // retina index
            this->branches[i].tz = {this->retina->d_x[ri], this->retina->d_y[ri]};
            //std::cout << "tzone: d_x[" << ri << "]";
            // Set its ephrin interaction parameters (though these may be related to the tz)
            this->branches[i].EphA = T{1.05} + (T{0.26} * std::exp (T{2.3} * this->retina->d_x[ri])); // R(x) = 0.26e^(2.3x) + 1.05,
            // Set its initial location randomly
            morph::Vector<T, 2> initpos = { rn[2*i], rn[2*i+1] };
            this->branches[i].path.clear();
            this->branches[i].path.push_back (initpos);
            this->branches[i].id = i;
        }
        // Parameters settable from json
        this->m[0] = this->conf->getDouble ("m1", 0.02);
        this->m[1] = this->conf->getDouble ("m2", 0.2);
        this->m[2] = this->conf->getDouble ("m3", 0.15);
        this->m[3] = this->conf->getDouble ("mborder", 0.1);

        // Visualization init
        const unsigned int ww = this->conf->getUInt ("win_width", 800);
        unsigned int wh = static_cast<unsigned int>(0.8824f * (float)ww);
        this->v = new morph::Visual (ww, wh, "Simpson-Goodhill extended XBAM");
        this->v->backgroundWhite();
        this->v->lightingEffects();

        // Offset for visuals
        morph::Vector<float> offset = { -0.5f, -0.5f, 0.0f };

        //
        this->bv = new BranchVisual<T> (v->shaderprog, offset, &this->branches);
        this->bv->finalize();
        v->addVisualModel (this->bv);

        // Show a vis of the retina, to compare positions/colours
        offset[0] += 3.0f;
        offset[1] += 0.5f;
        morph::CartGridVisual<float>* cgv = new morph::CartGridVisual<float>(v->shaderprog, v->tshaderprog, retina, offset);
        cgv->cartVisMode = morph::CartVisMode::RectInterp;
        std::vector<morph::Vector<float, 3>> points = this->retina->getCoordinates3();
        cgv->setVectorData (&points);
        cgv->cm.setType (morph::ColourMapType::Duochrome);
        cgv->cm.setHueRG();
        cgv->finalize();
        v->addVisualModel (cgv);
    }

    std::vector<T> ephcolourdata;
    std::vector<morph::Vector<float, 3>> rgcposcolourdata;
    std::vector<morph::Vector<float, 3>> coords;

    // Branches per axon
    static constexpr unsigned int bpa = 8;
    // Number of RGCs on a side
    static constexpr unsigned int rgcside = 20;
    // Access to a parameter configuration object
    morph::Config* conf;
    // rgcside^2 RGCs, each with bpa axon branches growing.
    morph::CartGrid* retina;
    // Parameters vecto (See Table 2 in the paper)
    morph::Vector<T, 4> m = { T{0.02}, T{0.2}, T{0.15}, T{0.1} };
    // The centre coordinate
    morph::Vector<T,2> centre = { T{0.5}, T{0.5} }; // FIXME get from CartGrid
    // (rgcside^2 * bpa) branches, as per the paper
    std::vector<branch<T>> branches;
    // A visual environment
    morph::Visual* v;
    // Specialised visualization of agents with a history
    BranchVisual<T>* bv;
};

int main (int argc, char **argv)
{
    // Set up config object
    std::string paramsfile;
    if (argc >= 2) {
        paramsfile = std::string(argv[1]);
    } else {
        // Create an empty/default json file
        paramsfile = "sg.json";
        morph::Tools::copyStringToFile ("{ \"steps\"=1000 }\n", paramsfile);
    }

    morph::Config conf(paramsfile);
    if (!conf.ready) {
        std::cerr << "Failed to read config " << paramsfile << ". Exiting.\n";
        return 1;
    }
    SimpsonGoodhill<float> model (&conf);
    model.run();

    return 0;
}
