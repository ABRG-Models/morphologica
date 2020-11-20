// Do with the new code what evolve_fit_genome.cpp does in AttractorScaffolding codebase
#include <morph/bn/GeneNetDual.h>

using std::endl;
using std::cout;

int main()
{
    static constexpr size_t n = 5;
    static constexpr size_t k = 5;

    morph::bn::GeneNetDual<n,k> gn;
    // Set intial states
    gn.state_ant = 0x0;
    gn.state_pos = 0x0;
    // Set targets
    gn.target_ant = 0x15;
    gn.target_pos = 0xa;

    morph::bn::Genome<n,k> g = gn.evolve_new_genome (0.05f);

    // Show genome
    std::cout << "Evolved genome:\n" << g << std::endl;

    return 0;
}
