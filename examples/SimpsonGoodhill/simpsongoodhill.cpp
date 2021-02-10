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

// Retinotectal axon branch
template<typename T>
struct branch
{
    void update()
    {
        morph::Vector<T, 2> G, C, I;
        G =
        morph::Vector<T, 2> newpos = path.back();
        newpos += (m[0] * G + m[1] * C + m[2] * I); // * v where v=1
    }

    // The location and all previous locations of this branch.
    std::vector<morph::Vector<T, 2>> path;
    // Termination zone for this branch
    morph::Vector<T, 2> tz;
    // Parameter vector (hardcoded)
    morph::Vector<T, 3> m = { T{1}, T{1}, T{1} };
};

template<typename T>
struct SimpsonGoodhill
{
    SimpsonGoodhill (morph::Config* cfg)
    {
        this->conf = cfg;
        this->init();
    }
    ~SimpsonGoodhhill() { delete this->retina; }

    void run()
    {
        for (auto& b8 : this->branches) {
            for (auto& b : b8) {
                // b is ref to type 'branch<T>'
                b.update();
            }
        }

        // Visualise here.
    }

    void init()
    {
        // gr is grid element length
        T gr = T{1}/T{20};
        this->retina = new morph::CartGrid(gr, gr, 1, 1);
        this->branches.resize(this->retina->num());
        // init all branches with relevant termination zone
    }

    // Access to aparamteres configuration
    morph::Config* conf;
    // 20x20 RGCs, each with 8 axon branches growing.
    morph::CartGrid* retina;
    // 20x20x8 branches, as per the paper
    std::vector<std::array<branch<T>, 8>> branches;
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
