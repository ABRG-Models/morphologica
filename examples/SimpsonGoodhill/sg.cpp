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
#include <morph/ScatterVisual.h>
#include <morph/CartGridVisual.h>

#include "branch.h"
//#include "branchvisual.h"

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
            if (i%100 == 0) { std::cout << "step " << i << "\n"; }
            this->step();
            this->vis();
        }
        std::cout << "Done simulating\n";
        this->v->keepOpen();
    }

    void vis()
    {
        glfwPollEvents();
        this->setScatter();
        this->sv->reinit();
        this->v->render();
    }

    void step()
    {
        // Compute the next position for each branch:
#pragma omp parallel for
        for (auto& b : this->branches) { b.compute_next (this->branches); }
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
        // Visualization init
        const unsigned int ww = this->conf->getUInt ("win_width", 800);
        unsigned int wh = static_cast<unsigned int>(0.8824f * (float)ww);
        this->v = new morph::Visual (ww, wh, "Simpson-Goodhill extended XBAM");
        this->v->lightingEffects();

        morph::Vector<float> offset = { -0.5f, -0.5f, 0.0f };
        this->sv = new morph::ScatterVisual<T> (v->shaderprog, offset);
        this->sv->radiusFixed = 0.01f;
        this->sv->cm.setType (morph::ColourMapType::Duochrome);
        this->sv->cm.setHueRG();
        this->setScatter();
        this->sv->finalize();
        v->addVisualModel (this->sv);

        // Show a vis of the retina, to compare positions/colours. Duh, Use CartgridVisual!
        offset[0] += 2.2f;
#if 1
        morph::ScatterVisual<float>* svr = new morph::ScatterVisual<float> (v->shaderprog, offset);
        std::vector<morph::Vector<float, 3>> points = this->retina->getCoordinates3();
        svr->setDataCoords (&points);
        //sv->setScalarData (&data);
        // Set the vector data to the coordinates - we'll visualize duochrome based on x and y
        svr->setVectorData (&points);
        svr->radiusFixed = 0.035f;
        //svr->colourScale = scale;
        svr->cm.setType (morph::ColourMapType::Duochrome);
        svr->cm.setHueRG();
        svr->finalize();
        v->addVisualModel (svr);
#else
        // Need to scale the vectorData into correct colour values to get the CartGridVisual to work
        morph::CartGridVisual<float>* cgv = new morph::CartGridVisual<float>(v->shaderprog, v->tshaderprog, retina, offset);
        cgv->cartVisMode = morph::CartVisMode::RectInterp;
        std::vector<morph::Vector<float, 3>> points = this->retina->getCoordinates3();
        cgv->setVectorData (&points);
        cgv->cm.setType (morph::ColourMapType::Duochrome);
        cgv->cm.setHueRG();
        cgv->finalize();
        v->addVisualModel (cgv);
#endif
    }

    std::vector<T> ephcolourdata;
    std::vector<morph::Vector<float, 3>> rgcposcolourdata;
    std::vector<morph::Vector<float, 3>> coords;

    void setScatter()
    {
        this->rgcposcolourdata.clear();
        this->ephcolourdata.clear();
        this->coords.clear();
        for (auto& b : this->branches) {
            ephcolourdata.push_back (b.EphA);
            rgcposcolourdata.push_back ( {b.tz.x(), b.tz.y(), 0} );
            coords.push_back ({b.path.back().x(), b.path.back().y(), 0});
        }
        this->sv->setScalarData (&this->ephcolourdata);
        this->sv->setVectorData (&this->rgcposcolourdata);
        this->sv->setDataCoords (&this->coords);
    }

    // Branches per axon
    static constexpr unsigned int bpa = 1;
    // Number of RGCs on a side
    static constexpr unsigned int rgcside = 1;
    // Access to a parameter configuration object
    morph::Config* conf;
    // rgcside^2 RGCs, each with bpa axon branches growing.
    morph::CartGrid* retina;
    // The centre coordinate
    morph::Vector<T,2> centre = { T{0.5}, T{0.5} }; // FIXME get from CartGrid
    // (rgcside^2 * bpa) branches, as per the paper
    std::vector<branch<T>> branches;
    // A visual environment
    morph::Visual* v;
    // Scatter plot for the visualization
    morph::ScatterVisual<T>* sv;
};

int main (int argc, char **argv)
{
    // Set up config object
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " /path/to/params.json" << std::endl;
        return 1;
    }
    std::string paramsfile (argv[1]);
    morph::Config conf(paramsfile);
    if (!conf.ready) {
        std::cerr << "Failed to read config " << paramsfile << ". Exiting.\n";
        return 1;
    }

    SimpsonGoodhill<float> model (&conf);
    model.run();

    return 0;
}
