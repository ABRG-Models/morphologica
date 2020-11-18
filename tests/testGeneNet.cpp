#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>

using std::endl;
using std::cout;

int main()
{
    // Flip probability
    float p = 0.01f;

    // Note: compile time constants - i.e. not just const from early on in the program
    const size_t n = 5;
    const size_t k = 5;

    cout << "N=" << n << ", K=" << k << endl;
    morph::bn::Genome<n, k> g;
    g.randomize();
#if 0
    cout << "Genome 1:            " << g << endl;
    // Get a copy:
    morph::bn::Genome<n, k> g2 = g;
    cout << "Genome 2 after copy: " << g2 << endl;

    g2.evolve (p);
    cout << "Genome 2 evolved:    " << g2 << endl;
    cout << "Hamming distance between them: " << g.hamming(g2) << endl;

    g.evolve (p);
    cout << "Genome 1 evolved:    " << g << endl;
#endif
    morph::bn::GeneNet<n, k> gn;
    gn.p = p;
    gn.state = 0x2;
    cout << "Gene net initial state:\n"
         << morph::bn::GeneNet<n,k>::state_table(gn.state) << endl;
    // Develop according to g
    for (size_t i = 0; i < 1; ++i) {
        gn.develop(g);
        cout << "Gene net state is now:  " << morph::bn::GeneNet<n,k>::state_str(gn.state) << endl;
    }
    cout << g.table() << endl;
    return 0;
}
