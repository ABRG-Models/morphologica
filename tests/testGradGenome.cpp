#include <morph/bn/GradGenome.h>

using std::endl;
using std::cout;

// Note: compile time constants - i.e. not just const from early on in the program
const size_t n = 2;

// Globally initialise Random instance pointer - necessary for all progs using Genome
template<> morph::bn::Random<n,n>* morph::bn::Random<n,n>::pInstance = 0;

int main()
{
    morph::bn::GradGenome<n> gg;
    gg.randomize();
    cout << "Random gradient Genome:  " << gg << endl;

    gg.set ("3-4");

    cout << "For Gradient genome " << gg << ":\n"
         << "Gene 0 climbs Gene 0? " << (gg.i_climbs_j (0,0) ? "true" : "false") << "\n"
         << "Gene 0 descends Gene 0? " << (gg.i_descends_j (0,0) ? "true" : "false") << "\n"
         << "Gene 0 climbs Gene 1? " << (gg.i_climbs_j (0,1) ? "true" : "false") << "\n"
         << "Gene 0 descends Gene 1? " << (gg.i_descends_j (0,1) ? "true" : "false") << "\n"
         << "Gene 1 climbs Gene 1? " << (gg.i_climbs_j (1,1) ? "true" : "false") << "\n"
         << "Gene 1 descends Gene 1? " << (gg.i_descends_j (1,1) ? "true" : "false") << "\n"
         << "Gene 1 climbs Gene 0? " << (gg.i_climbs_j (1,0) ? "true" : "false") << "\n"
         << "Gene 1 descends Gene 0? " << (gg.i_descends_j (1,0) ? "true" : "false") << "\n";

    std::string sd = (gg.degenerate() ? "true":"false");
    cout << "Degenerate? " << sd << std::endl;
    cout << "Self degenerate? " << (gg.selfdegenerate() ? "true":"false") << std::endl;

    cout << endl << gg.table();

    return 0;
}
