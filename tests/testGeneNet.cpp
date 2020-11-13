#include <morph/bn/Genome.h>

int main()
{
    morph::bn::Genome<unsigned int, 5, 5> g;
    g.randomize();
    std::cout << "Genome 1: " << g << std::endl;
    morph::bn::Genome<unsigned int, 5, 5> g2; // can't do = g as my RandUniform class needs a copy constructor
    g2.copyin (g);
    std::cout << "Genome 2 before: " << g2 << std::endl;
    g2.flip_probability = 0.01f;
    g2.evolve();
    std::cout << "Genome 2 evolved: " << g2 << std::endl;
    std::cout << "Hamming distance between them: " << g.hamming(g2) << std::endl;
    std::cout << g2.table();
    return 0;
}
