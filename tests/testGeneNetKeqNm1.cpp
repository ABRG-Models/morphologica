#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>

using std::endl;
using std::cout;

// Note: compile time constants - i.e. not just const from early on in the program
const size_t n = 5;
const size_t k = 5;
// Globally initialise Random instance pointer - necessary for all progs using Genome
template<> morph::bn::Random<n,k>* morph::bn::Random<n,k>::pInstance = 0;

int main()
{
    cout << "N=" << n << ", K=" << k << endl;
    morph::bn::Genome<n, k> g;
    g.randomize();
    morph::bn::GeneNet<n, k> gn;
    morph::bn::state_t state = 0x2;
    cout << "Gene net initial state:\n"
         << morph::bn::GeneNet<n,k>::state_table(state) << endl;
    // Develop according to g
    gn.develop (state, g);
    cout << "Gene net state is now:  " << morph::bn::GeneNet<n,k>::state_str(state) << endl;
    // Display the Genome table
    cout << g.table() << endl;

    // Not absolutely necessary, but this call cleans up singleton memory for valgrind
    // memcheck. Would ideally go in the Genome deconstructor, though this is not
    // possible because morph::bn::Genome derives from the union std::array
    morph::bn::Random<n,k>::i_deconstruct();

    return 0;
}
