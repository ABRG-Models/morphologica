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
    const size_t k = 4;

    cout << "N=" << n << ", K=" << k << endl;
    morph::bn::Genome<n, k> g;
    g.randomize();
    morph::bn::GeneNet<n, k> gn;
    gn.p = p;
    gn.state = 0x2;
    cout << "Gene net initial state:\n"
         << morph::bn::GeneNet<n,k>::state_table(gn.state) << endl;
    // Develop according to g
    gn.develop(g);
    cout << "Gene net state is now:  " << morph::bn::GeneNet<n,k>::state_str(gn.state) << endl;
    // Display the Genome table
    cout << g.table() << endl;

    return 0;
}
