#include <morph/bn/Genome.h>

int main()
{
    morph::bn::Genome<unsigned int, 5, 5> g;
    g.randomize();
    std::cout << "Genome 1: " << g << std::endl;
    // Get a copy:
    morph::bn::Genome<unsigned int, 5, 5> g2 = g;
    std::cout << "Genome 2 after copy: " << g2 << std::endl;

    g2.p = 0.01f;
    g2.evolve();
    std::cout << "Genome 2 evolved: " << g2 << std::endl;
    std::cout << "Hamming distance between them: " << g.hamming(g2) << std::endl;

    g.p = 0.01f;
    g.evolve();
    std::cout << "Genome 1 evolved: " << g << std::endl;

    return 0;
}
