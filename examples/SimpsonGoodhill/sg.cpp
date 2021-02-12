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
    // Compute the next position for this branch, using information from all other branches
    void compute_next (const std::vector<std::array<branch<T>, 8>>& branches)
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
                if (b.id == this->id) { continue; } // Don't interact with self
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

        // Border effect. A force perpendicular to the boundary, falling off over the
        // distance r.
        morph::Vector<T, 2> B = {0, 0};
        // Test k, to see if it's near the border. Use winding number to see if it's
        // inside? Then, if outside, find out which edge it's nearest and apply that
        // force. Too complex. Instead, look at k's location. If x<0, then add component
        // to B[0]; if y<0 then add component to B[1], etc.
        if (k[0] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{1};
        } else if (k[0] < r) {
            B[0] = T{1} * (T{1} - k[0]/r); // B[0] prop 1 - k/r
        } else if (k[0] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[0] = T{-1};
        } else if (k[0] > (1-r)) {
            B[0] = -(k[0] + r - T{1})/r; // B[0] prop (k+r-1)/r
        }

        if (k[1] < T{0}) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{1};
        } else if (k[1] < r) {
            B[1] = T{1} - k[1]/r;
        } else if (k[1] > 1) {
            G = {0,0};
            I = {0,0};
            C = {0,0};
            B[1] = T{-1};
        } else if (k[1] > (1-r)) {
            B[1] = -(k[1] + r - T{1})/r; // B[1] prop (k+r-1)/r
        }

        // Paper equation 1
        k += (G * m[0] + C * m[1] + I * m[2] + B * m[3]); // * v where v=1
        this->next = k;
    }
    // The location and all previous locations of this branch.
    std::vector<morph::Vector<T, 2>> path;
    // Place the next computed location for path in 'next' so that while computing, we
    // don't modify the numbers we're working from. After looping through all branches,
    // add this to path.
    morph::Vector<T, 2> next;
    // Termination zone for this branch
    morph::Vector<T, 2> tz = {0, 0};
    // EphA expression for this branch
    T EphA = 0;
    // A sequence id
    int id = 0;
    // Parameter vector (hardcoded, see Table 2 in paper) where here, m[3] (the last
    // element) is the border effect magnitude.
    static constexpr morph::Vector<T, 4> m = { T{0.02}, T{0.2}, T{0.15}, T{0.1} };
    // Distance parameter r is used as 2r
    static constexpr T two_r = T{0.1};
    static constexpr T r = T{0.05};
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
        // Compute the next position for each branch
        for (auto& b8 : this->branches) {
            for (auto& b : b8) { b.compute_next (this->branches); }
        }
        // Once 'next' has been updated, add next to path
        for (auto& b8 : this->branches) {
            for (auto& b : b8) { b.path.push_back (b.next); }
        }
    }

    void init()
    {
        // Simulation init
        morph::RandUniform<T, std::mt19937> rng;
        // gr is grid element length
        T gr = T{1}/T{20};
        std::cout << "Grid element length " << gr << std::endl;
        this->retina = new morph::CartGrid(gr, gr, 1, 1);
        this->retina->setBoundaryOnOuterEdge();
        std::cout << "Retina has " << this->retina->num() << " cells\n";
        this->branches.resize(this->retina->num());
        // init all branches with relevant termination zone
        int ret_idx = 0;
        int br_idx = 0;
        std::vector<T> rn = rng.get (this->retina->num() * 2 * 8);
        for (auto& b8 : this->branches) {
            branch<T> b;
            // Set the branch's termination zone
            b.tz = {this->retina->d_x[ret_idx], this->retina->d_y[ret_idx]};
            // Set its ephrin interaction parameters (though these may be related to the tz)
            b.EphA = T{1.05} + (T{0.26} * std::exp (T{2.3} * this->retina->d_x[ret_idx])); // R(x) = 0.26e^(2.3x) + 1.05,
            // Set its initial location randomly
            for (size_t j = 0; j < 8; ++j) {
                morph::Vector<T, 2> initpos = { rn[ret_idx+2*j], rn[ret_idx+2*j+1] };
                b.path.clear();
                b.path.push_back (initpos);
                b.id = br_idx++;
                b8[j] = b;
            }
            ++ret_idx;
        }
        // Visualization init
        const unsigned int ww = this->conf->getUInt ("win_width", 800);
        unsigned int wh = static_cast<unsigned int>(0.8824f * (float)ww);
        this->v = new morph::Visual (ww, wh, "Simpson-Goodhill extended XBAM");
        this->v->lightingEffects();

        morph::Vector<float> offset = { -0.5f, -0.5f, 0.0f };
        this->sv = new morph::ScatterVisual<T> (v->shaderprog, offset);
        this->sv->radiusFixed = 0.01f;
        this->sv->cm.setType (morph::ColourMapType::Jet);
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
    // The centre coordinate
    morph::Vector<T,2> centre = { T{0.5}, T{0.5} }; // FIXME get from CartGrid
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
