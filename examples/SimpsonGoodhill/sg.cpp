/*
 * Reimplementation of retinotectal model presented by Hugh Simpson and Geoffrey
 * Goodhill in "A simple model can unify a broad range of phenomena in retinotectal map
 * development", Biol Cybern (2011) 104:9-29
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include <morph/vec.h>
#include <morph/CartGrid.h>
#include <morph/Config.h>
#include <morph/Random.h>
#include <morph/Visual.h>
#include <morph/CartGridVisual.h>

#include "branchvisual.h"
#include "branch.h"
#include "netvisual.h"
#include "net.h"

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
            this->vis(i);
            if (i%100 == 0) { std::cout << "step " << i << "\n"; }
        }
        std::cout << "Done simulating\n";
        this->v->keepOpen();
    }

    void vis(unsigned int stepnum)
    {
        if (this->goslow == true) {
            v->waitevents (0.1); // to add artificial slowing
        } else {
            v->poll();
        }
        this->bv->reinit();
        this->cv->reinit();
        this->v->render();
        if (this->conf->getBool ("movie", false)) {
            std::stringstream frame;
            frame << "frames/";
            frame.width(4);
            frame.fill('0');
            frame << stepnum;
            frame << ".png";
            this->v->saveImage(frame.str());
        }
    }

    void step()
    {
        // Compute the next position for each branch:
#pragma omp parallel for
        for (unsigned int i = 0; i < this->branches.size(); ++i) {
            this->branches[i].compute_next (this->branches, this->m);
        }
        // Update centroids
        for (unsigned int i = 0; i < this->retina->num(); ++i) { this->ax_centroids.p[i] = {T{0}, T{0}, T{0}}; }
        for (auto& b : this->branches) {
            unsigned int ri = b.id/this->bpa;
            this->ax_centroids.p[ri][0] += b.next[0] / static_cast<T>(this->bpa);
            this->ax_centroids.p[ri][1] += b.next[1] / static_cast<T>(this->bpa);
        }
        // Once 'next' has been updated, add next to path:
        for (auto& b : this->branches) {
            b.path.push_back (b.next);
            if (b.path.size() > this->history) { b.path.pop_front(); }
        }
    }

    void init()
    {
        // Simulation init
        this->rgcside = this->conf->getUInt ("rgcside", this->rgcside);
        this->bpa = this->conf->getUInt ("bpa", 8);
        this->goslow = this->conf->getBool ("goslow", false);
        // gr is grid element length
        T gr_denom = rgcside-1;
        T gr = T{1}/gr_denom;
        std::cout << "Grid element length " << gr << std::endl;
        this->retina = new morph::CartGrid(gr, gr, 0.0f, 0.0f, 0.95f, 0.95f);
        this->retina->setBoundaryOnOuterEdge();
        std::cout << "Retina has " << this->retina->num() << " cells\n";
        this->branches.resize(this->retina->num() * bpa);

        std::cout << "Retina is " << this->retina->widthnum() << " wide and " << this->retina->depthnum() << " high\n";
        this->ax_centroids.init (this->retina->widthnum(), this->retina->depthnum());
        // Axon initial positions x and y are uniformly randomly selected
        morph::RandUniform<T, std::mt19937> rng_x(T{0}, T{1.0});
        morph::RandUniform<T, std::mt19937> rng_y(T{-0.2}, T{0});
        // A normally distributed perturbation is added for each branch. SD=0.1.
        morph::RandNormal<T, std::mt19937> rng_p(T{0}, T{0.1});
        // Generate random number sequences all at once
        std::vector<T> rn_x = rng_x.get (this->retina->num());
        std::vector<T> rn_y = rng_y.get (this->retina->num());
        std::vector<T> rn_p = rng_p.get (this->retina->num() * 2 * bpa);
        T EphA_max = -1e9;
        T EphA_min = 1e9;
        for (unsigned int i = 0; i < this->branches.size(); ++i) {
            // Set the branch's termination zone
            unsigned int ri = i/bpa; // retina index
            this->branches[i].tz = {this->retina->d_x[ri], this->retina->d_y[ri]};
            // Set its ephrin interaction parameters (though these may be related to the tz)
            this->branches[i].EphA = T{1.05} + (T{0.26} * std::exp (T{2.3} * this->retina->d_x[ri])); // R(x) = 0.26e^(2.3x) + 1.05,
            EphA_max =  this->branches[i].EphA > EphA_max ? branches[i].EphA : EphA_max;
            EphA_min =  this->branches[i].EphA < EphA_min ? branches[i].EphA : EphA_min;
            // Set as in the authors' paper - starting at bottom in region x=(0,1), y=(-0.2,0)
            morph::vec<T, 3> initpos = { rn_x[ri] + rn_p[2*i], rn_y[ri] + rn_p[2*i+1], 0 };
            morph::vec<T, 2> initpos2 = { rn_x[ri] + rn_p[2*i], rn_y[ri] + rn_p[2*i+1] };
            this->ax_centroids.p[ri] += initpos / static_cast<T>(bpa);
            this->branches[i].path.clear();
            this->branches[i].path.push_back (initpos2);
            this->branches[i].id = i;
        }
        // The min/max of EphA is used below to set a morph::Scale in branchvisual
        std::cout << "EphA range: " << EphA_min << " to " << EphA_max << std::endl;

        // Parameters settable from json
        this->m[0] = this->conf->getDouble ("m1", 0.02);
        this->m[1] = this->conf->getDouble ("m2", 0.2);
        this->m[2] = this->conf->getDouble ("m3", 0.15);
        this->m[3] = this->conf->getDouble ("mborder", 0.1);

        // Visualization init
        const unsigned int ww = this->conf->getUInt ("win_width", 1200);
        unsigned int wh = static_cast<unsigned int>(0.5625f * (float)ww);
        std::cout << "New morph::Visual with width/height: " << ww << "/" << wh << std::endl;
        this->v = new morph::Visual (ww, wh, "Simpson-Goodhill extended XBAM");
        this->v->backgroundWhite();
        this->v->lightingEffects();

        // Offset for visuals
        morph::vec<float> offset = { -1.5f, -0.5f, 0.0f };

        // Visualise the branches with a custom VisualModel
        auto bvup = std::make_unique<BranchVisual<T>> (offset, &this->branches);
        v->bindmodel (bvup);
        bvup->EphA_scale.compute_autoscale (EphA_min, EphA_max);
        bvup->addLabel ("Branches", {0.0f, 1.1f, 0.0f});
        bvup->finalize();
        this->bv = v->addVisualModel (bvup);

        // Centroids of branches viewed with a NetVisual
        offset[0] += 1.3f;
        auto cvup = std::make_unique<NetVisual<T>> (offset, &this->ax_centroids);
        v->bindmodel (cvup);
        cvup->addLabel ("Axon centroids", {0.0f, 1.1f, 0.0f});
        cvup->finalize();
        this->cv = v->addVisualModel (cvup);

        // Show a vis of the retina, to compare positions/colours
        offset[0] += 1.3f;
        auto cgv = std::make_unique<morph::CartGridVisual<float>>(retina, offset);
        v->bindmodel (cgv);
        cgv->cartVisMode = morph::CartVisMode::RectInterp;
        std::vector<morph::vec<float, 3>> points = this->retina->getCoordinates3();
        cgv->setVectorData (&points);
        cgv->cm.setType (morph::ColourMapType::Duochrome);
        cgv->cm.setHueRG();
        cgv->addLabel ("Retina", {0.0f, 1.1f, 0.0f});
        cgv->finalize();
        v->addVisualModel (cgv);
    }

    std::vector<T> ephcolourdata;
    std::vector<morph::vec<float, 3>> rgcposcolourdata;
    std::vector<morph::vec<float, 3>> coords;

    // Branches per axon
    unsigned int bpa = 8;
    // Number of RGCs on a side
    unsigned int rgcside = 21;
    // If true, then slow things down a bit in the visualization
    bool goslow = false;
    // How many steps to store/show history?
    static constexpr size_t history = 20;
    // Access to a parameter configuration object
    morph::Config* conf;
    // rgcside^2 RGCs, each with bpa axon branches growing.
    morph::CartGrid* retina;
    // Parameters vecto (See Table 2 in the paper)
    morph::vec<T, 4> m = { T{0.02}, T{0.2}, T{0.15}, T{0.1} };
    // The centre coordinate
    morph::vec<T,2> centre = { T{0.5}, T{0.5} }; // FIXME get from CartGrid
    // (rgcside^2 * bpa) branches, as per the paper
    std::vector<branch<T>> branches;
    // Centroid of the branches for each axon
    net<T> ax_centroids;
    // A visual environment
    morph::Visual* v;
    // Specialised visualization of agents with a history
    BranchVisual<T>* bv;
    // Centroid visual
    NetVisual<T>* cv;
};

int main (int argc, char **argv)
{
    // Set up config object
    std::string paramsfile;
    if (argc >= 2) {
        paramsfile = std::string(argv[1]);
    } else {
        // Create an empty/default json file
        paramsfile = "./sg.json";
        morph::Tools::copyStringToFile ("{}\n", paramsfile);
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
