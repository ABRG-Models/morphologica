// Do with the new code what evolve.cpp does in AttractorScaffolding codebase
#include <string>
#include <limits>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>

#include <morph/Config.h>

#include <morph/bn/GeneNetDual.h>
#include <morph/bn/Genome.h>
#include <morph/bn/Random.h>

// The number of generations to evolve for by default, unless otherwise specfied in JSON
#define N_Generations   100000000

using morph::bn::state_t;

struct geninfo {
    geninfo (unsigned long long int _gen, unsigned long long int _gen_0, double _fit)
        : gen(_gen)
        , gen_0(_gen_0)
        , fit(_fit)
        {}
    unsigned long long int gen;   // generations since last increase in fitness
    unsigned long long int gen_0; // generation since last F=1
    double fit;                   // The fitness
};

//! Globally initialise Random instance pointer
template<> morph::bn::Random<5,5>* morph::bn::Random<5,5>::pInstance = 0;

int main (int argc, char** argv)
{
    // Get JSON parameter config path
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " /path/to/params.json [p]"
                  << "       (p may be specified to override any value obtained from the JSON)"
                  << std::endl;
        return 1;
    }
    std::string paramsfile (argv[1]);

    // Allow p to be specified on the command line, overriding the JSON config
    float pCmd = -1.0f;
    if (argc >= 3) {
        pCmd = static_cast<float>(atof (argv[2]));
        std::cout << "p in JSON will be overridden to " << pCmd << std::endl;
    }

    morph::Config conf(paramsfile);
    if (!conf.ready) {
        std::cerr << "Error setting up JSON config: " << conf.emsg << std::endl;
        return 1;
    }

    // Set up simulation parameters from JSON (or command line, if overridden)
    float p = pCmd > -1.0f ? pCmd : conf.getFloat ("p", 0.5f);

    // How many generations in total to evolve for (counting f=1 genomes as you go)
    unsigned long long int nGenerations =
    static_cast<unsigned long long int>(conf.getUInt ("nGenerations", N_Generations));

    // If set to >0, then when this number of fit genomes have been found, finish
    // then. In this case, set nGenerations to unsigned long long int max
    const unsigned int finishAfterNFit = conf.getUInt ("finishAfterNFit", 0);
    if (finishAfterNFit > 0) {
        nGenerations = std::numeric_limits<unsigned long long int>::max();
    }

    // How often to output a progress message on stdout
    const unsigned int nGenView = conf.getUInt ("nGenView", N_Generations/100);

    // Where to save out the logs
    const std::string logdir = conf.getString ("logdir", "./data");
    // Should we append data to the given file, rather than overwriting?
    const bool append_data = conf.getBool ("append_data", false);

    static constexpr size_t n = 5;
    static constexpr size_t k = 5;

    // generations records the relative generation number, and the fitness. Every entry
    // in this records an increase in the fitness of the genome.
    std::vector<geninfo> generations;

    // Holds the genome and a copy of it.
    morph::bn::Genome<n,k> refg;
    morph::bn::Genome<n,k> newg;

    // The main loop. Repeatedly evolve from a random genome starting point, recording the number
    // of generations required to achieve a maximally fit state of 1.
    unsigned long long int gen = 0;
    unsigned long long int lastgen = 0;
    unsigned long long int lastf1 = 0;

    // Count F=1 genomes to print out at the end.
    unsigned long long int f1count = 0;

    // Set the fitness threshold at which we say the system is fully fit. For synchronous
    // development, this should be exactly 1.
    double fitness_threshold = 1.0;

    morph::bn::GeneNetDual<n,k> gn;
    gn.state_ant = 0x0;
    gn.state_pos = 0x0;
    gn.target_ant = 0x15;
    gn.target_pos = 0xa;
    unsigned int nContexts = 2;

    morph::bn::Genome<n,k> g = gn.evolve_new_genome (0.05f);

    // Show genome
    std::cout << "Evolved genome:\n" << g << std::endl;

    while (gen < nGenerations && (finishAfterNFit==0 || f1count < finishAfterNFit)) {

        // At the start of the loop, and every time fitness of 1.0 is achieved, generate
        // a random genome starting point.
        refg.randomize();

        // Make a copy of the genome, in case evolving it leads to a less fit genome, then
        // evaluate the fitness of the genome.
        double a = gn.evaluate_fitness (refg);

        // a randomly selected genome can be maximally fit
        if (a>=fitness_threshold) {
            generations.push_back (geninfo(gen-lastgen, gen-lastf1, a));
            lastgen = gen;
            lastf1 = gen;
            ++f1count;
        }

        ++gen; // Because we randomly generated.

        // Note that if a==1.0 after the call to random_genome(), we should cycle around
        // and call randomize() again.

        // Test fitness to determine whether we should evolve
        while (a < fitness_threshold) {
            newg = refg;
            newg.mutate (p);
            ++gen; // Because we mutated

            if (gen > 0 && (gen % nGenView == 0)) {
                std::cout << "[p=" << p << "] That's " << gen/1000000.0 << "M generations (out of "
                          << nGenerations/1000000.0 << "M) done...\n";
            }

            if (gen >= nGenerations) { break; }

            double b = gn.evaluate_fitness (newg);

            // DRIFT: Old fitness >= new fitness
            if (b >= a) {
                // Record the fitness increase in generations:
                if (b>=fitness_threshold) {
                    generations.push_back (geninfo(gen-lastgen, gen-lastf1, b));
                }
                lastgen = gen;
                if (b>=fitness_threshold) {
                    lastf1 = gen;
                    DBG ("F=1 at generation " << gen);
                    ++f1count;
                }
                // Copy new fitness to ref
                a = b;
                // Copy new to reference
                refg = newg;
            }
        }
    }

    std::cout << "Generations size: " << generations.size()
              << " with " << f1count << " F=1 genomes found.\n";

    // Save data to file.
    std::ofstream f, f1;
    std::stringstream pathss;
    pathss << logdir << "/";
    pathss << "evolve_";
    pathss << "nc" << nContexts;
#if 0
    pathss << "_I";
    for (unsigned int i = 0; i < nContexts; ++i) {
        if (i) { pathss << "-"; }
        pathss << (unsigned int)initials[i];
    }
    pathss << "_T";
    for (unsigned int i = 0; i < nContexts; ++i) {
        if (i) { pathss << "-"; }
        pathss << (unsigned int)targets[i];
    }
#endif
    pathss << "_";

    if (finishAfterNFit == 0) {
        pathss << "ASff4" << "_" << nGenerations << "_gens_" << p << ".csv";
    } else {
        pathss << "ASff4" << "_" << finishAfterNFit << "_fits_" << p << ".csv";
    }

    if (append_data == true) {
        f.open (pathss.str().c_str(), std::ios::out|std::ios::app);
    } else {
        f.open (pathss.str().c_str(), std::ios::out|std::ios::trunc);
    }
    if (!f.is_open()) {
        std::cerr << "Error opening " << pathss.str() << std::endl;
        return 1;
    }

    for (unsigned int i = 0; i < generations.size(); ++i) {
        // One file has the time taken to get to F=1
        if (generations[i].fit >= fitness_threshold) {
            f << generations[i].gen_0 << std::endl;
        }
    }
    f.close();

    return 0;
}
