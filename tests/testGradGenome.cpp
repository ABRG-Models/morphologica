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
    cout << "Self degenerate? " << (gg.selfdegenerate() ? "true":"false") << endl;

    cout << endl << gg.table();

    cout << "\nCycle through a full gradient genome...\n";

    gg.set ("0-0");
    cout << gg << endl;
    unsigned int num = 0;
    unsigned int num_nondegen = 0;
    while (gg.inc()) {
        // It's a new genome
        ++num;
        std::string s = "";
        if (gg.degenerate()) {
            s += " degenerate";
            if (gg.selfdegenerate()) {
                s += " self-degenerate";
            }
        } else {
            if (gg.selfdegenerate()) {
                s += " self-degenerate";
            } else {
                s += " non-degenerate";
                ++num_nondegen;
            }
        }
        cout << gg << s << endl;
    }
    cout << "Num possibles: " << num << ", num non-degenerate: " << num_nondegen << endl;
    return 0;
}
