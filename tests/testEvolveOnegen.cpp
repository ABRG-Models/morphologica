// Do with the new code what evolve_onegen.cpp does in AttractorScaffolding codebase
#include <morph/bn/GeneNetDual.h>
#include <morph/bn/Genome.h>
#include <morph/bn/Random.h>

using std::endl;
using std::cout;

static constexpr size_t n = 5;
static constexpr size_t k = 5;
// Globally initialise Random instance pointer - necessary for all progs using Genome
template<> morph::bn::Random<n,k>* morph::bn::Random<n,k>::pInstance = 0;

int main()
{
    morph::bn::GeneNetDual<n,k> gn;
    // Set intial states
    gn.state_ant = 0x0;
    gn.state_pos = 0x0;
    // Set targets
    gn.target_ant = 0x15;
    gn.target_pos = 0xa;

    morph::bn::Genome<n,k> g;
    gn.set_selected (g); // Our special, selected genome

    double f = gn.evaluate_fitness (g);
    std::cout << "Genome:\n" << g << std::endl;
    std::cout << "BEFORE The fitness of the selected genome is " << f << std::endl;
    g.mutate (0.07f);
    f = gn.evaluate_fitness (g);
    std::cout << "AFTER The fitness of the selected genome is " << f << std::endl;
    std::cout << "and canalyzingness is " << g.canalyzingness() << std::endl;
    // Show genome
    std::cout << "Mutated genome:\n" << g << std::endl;

    // Not absolutely necessary, but this call cleans up singleton memory for valgrind
    // memcheck. Would ideally go in the Genome deconstructor, though this is not
    // possible because morph::bn::Genome derives from the union std::array
    morph::bn::Random<n,k>::i_deconstruct();

    return 0;
}
