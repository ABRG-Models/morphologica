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

// Retinotectal axon branch
template<typename T>
struct branch
{
    void update (const std::vector<std::array<branch<T>, 8>>& branches)
    {
        // Current location is named k
        morph::Vector<T, 2> k = path.back();
        // Chemoaffinity is G
        morph::Vector<T, 2> G = this->tz - k;
        // Competition, C, and Axon-axon interactions, I, computed during the same loop
        // over the other branches
        morph::Vector<T, 2> C = {0, 0};
        morph::Vector<T, 2> I = {0, 0};
        morph::Vector<T, 2> nvec = {0, 0}; // null vector
        for (auto b8 : branches) {
            for (auto b : b8) {
                morph::Vector<T, 2> bk = k - b.path.back();
                T d = bk.length();
                T W = d <= this->two_r ? (T{1} - d/this->two_r) : T{0};
                T Q = b.EphA / this->EphA; // forward signalling (used predominantly in paper)
                //T Q = this->EphA / b.EphA; // reverse signalling
                //T Q = std::max(b.EphA / this->EphA, this->EphA / b.EphA); // bi-dir signalling
                bk.renormalize();
                I += Q > this->s ? bk * W : nvec;
                C += bk * W;
            }
        }
        // Possibly do C -= self, as I looped over ALL branches, above (same for I)
        C.renormalize(); // achieves 1/|Bb|
        I.renormalize();

        // Paper equation 1
        k += (G * m[0] + C * m[1] + I * m[2]); // * v where v=1
        this->path.push_back (k);
    }
    // The location and all previous locations of this branch.
    std::vector<morph::Vector<T, 2>> path;
    // Termination zone for this branch
    morph::Vector<T, 2> tz = { T{0}, T{0} };
    // EphA expression for this branch
    T EphA = 0;
    // EphB expression for this branch
    T EphB = 0;
    // Parameter vector (hardcoded, see Table 2 in paper)
    static constexpr morph::Vector<T, 3> m = { T{0.02}, T{0.2}, T{0.15} };
    // Distance parameter r is used as 2r
    static constexpr T two_r = T{0.1};
    // Signalling ratio parameter
    static constexpr T s = T{1.1};
};

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
            std::cout << "step\n";
            this->step();
            std::cout << "vis\n";
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
        // Update each branch's position once
        for (auto& b8 : this->branches) {
            for (auto& b : b8) {
                b.update (this->branches);
            }
        }
    }

    void init()
    {
        // Simulation init
        morph::RandUniform<T, std::mt19937> rng;
        // gr is grid element length
        T gr = T{1}/T{19};
        this->retina = new morph::CartGrid(gr, gr, 1, 1);
        this->retina->setBoundaryOnOuterEdge();
        std::cout << "Retina has " << this->retina->num() << " cells\n";
        this->branches.resize(this->retina->num());
        // init all branches with relevant termination zone
        size_t i = 0;
        std::vector<T> rn = rng.get (this->retina->num() * 2 * 8);
        for (auto& b8 : this->branches) {
            branch<T> b;
            // Set the branch's termination zone
            b.tz = {this->retina->d_x[i], this->retina->d_y[i]};
            // Set its ephrin interaction parameters (though these may be related to the tz)
            b.EphA = this->retina->d_x[i];
            b.EphB = this->retina->d_y[i];
            // Set its initial location randomly
            for (size_t j = 0; j < 8; ++j) {
                morph::Vector<T, 2> initpos = { rn[i+2*j], rn[i+2*j+1] };
                b.path.clear();
                b.path.push_back (initpos);
                b8[j] = b;
            }
            ++i;
        }
        // Visualization init
        const unsigned int ww = this->conf->getUInt ("win_width", 800);
        unsigned int wh = static_cast<unsigned int>(0.8824f * (float)ww);
        this->v = new morph::Visual (ww, wh, "Simpson-Goodhill extended XBAM");
        this->v->lightingEffects();

        morph::Vector<float> offset = { -0.5f, -0.5f, 0.0f };
        this->sv = new morph::ScatterVisual<T> (v->shaderprog, offset);
        this->sv->radiusFixed = 0.02f;
        this->setScatter();
        this->sv->finalize();
        v->addVisualModel (this->sv);
    }

    std::vector<T> colourdata;
    std::vector<morph::Vector<float, 3>> coords;

    void setScatter()
    {
        this->colourdata.clear();
        this->coords.clear();
        for (auto& b8 : this->branches) {
            for (size_t j = 0; j < 8; ++j) {
                colourdata.push_back (b8[j].EphA);
                coords.push_back ({b8[j].path.back().x(), b8[j].path.back().y(), 0});
            }
        }
        this->sv->setScalarData (&this->colourdata);
        this->sv->setDataCoords (&this->coords);
    }

    // Access to a parameter configuration object
    morph::Config* conf;
    // 20x20 RGCs, each with 8 axon branches growing.
    morph::CartGrid* retina;
    // 20x20x8 branches, as per the paper
    std::vector<std::array<branch<T>, 8>> branches;
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
