#include <morph/bn/Genome.h>

/*!
 * The genome has a section for each gene. The length of the section of each
 * gene is 2^K. 32 bit width sections are enough for N <= 6. 64 bit
 * width sections ok for N <= 7.
 */
#if 0 // Code in constexpr form that was previously preprocessor stuff
        if constexpr (N==7) {
            typedef unsigned long long int genosect_t;
        } else if constexpr (N==6) {
            if constexpr (N==K) {
                typedef unsigned long long int genosect_t;
            } else {
                typedef unsigned int genosect_t;
            }
        } else {
            typedef unsigned int genosect_t;
        }
#endif

int main()
{
    morph::bn::Genome<unsigned int, 5, 5> g;
    g.randomize();
    morph::bn::Genome<unsigned int, 5, 5> g2;
    g2.randomize();
    std::cout << "Genome 1: " << g << "\nGenome 2: " << g2 << std::endl;
    std::cout << "Hamming distance between them: " << g.hamming(g2) << std::endl;
    return 0;
}
