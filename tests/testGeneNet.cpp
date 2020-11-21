#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>
#include <morph/bn/GeneNetDual.h>

using std::endl;
using std::cout;

// Note: compile time constants - i.e. not just const from early on in the program
const size_t n = 5;
const size_t k = 5;

// Globally initialise Random instance pointer - necessary for all progs using Genome
morph::bn::Random<n,k>* morph::bn::Random<n,k>::pInstance = 0;

int main()
{
    morph::bn::Genome<n, k> g;
    g.randomize();
    morph::bn::GeneNet<n, k> gn;
    morph::bn::state_t state = 0x2;
    // Develop according to g
    for (size_t i = 0; i < 16; ++i) {
        gn.develop (state, g);
        cout << "Gene net state is now:  " << morph::bn::GeneNet<n,k>::state_str(state) << endl;
    }

    cout << g.table() << endl;

    morph::bn::GeneNetDual<n,k> gnd;
    gnd.state_ant = 0x8;
    gnd.state_pos = 0x2;
    gnd.develop (g);

    return 0;

#if 0
    cout << "Genome 1:            " << g << endl;
    // Get a copy:
    morph::bn::Genome<n, k> g2 = g;
    cout << "Genome 2 after copy: " << g2 << endl;

    g2.mutate (p);
    cout << "Genome 2 evolved:    " << g2 << endl;
    cout << "Hamming distance between them: " << g.hamming(g2) << endl;

    g.mutate (p);
    cout << "Genome 1 evolved:    " << g << endl;
#endif

}
