#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>

int main()
{
    // Flip probability
    float p = 0.01f;

    const size_t n = 5;
    const size_t k = 3;

    morph::bn::Genome<n, k> g;
    g.randomize();
#if 0
    std::cout << "Genome 1:            " << g << std::endl;
    // Get a copy:
    morph::bn::Genome<n, k> g2 = g;
    std::cout << "Genome 2 after copy: " << g2 << std::endl;

    g2.evolve (p);
    std::cout << "Genome 2 evolved:    " << g2 << std::endl;
    std::cout << "Hamming distance between them: " << g.hamming(g2) << std::endl;

    g.evolve (p);
    std::cout << "Genome 1 evolved:    " << g << std::endl;
#endif
    morph::bn::GeneNet<n, k> gn;
    gn.p = p;
    gn.state = 0x12; // example
    std::cout << "Gene net initial state: " << morph::bn::GeneNet<n,k>::state_str(gn.state) << std::endl;
    // Develop according to g
    for (size_t i = 0; i < 16; ++i) {
        gn.develop(g);
        std::cout << "Gene net state is now:  " << morph::bn::GeneNet<n,k>::state_str(gn.state) << std::endl;
    }
    std::cout << "Genome table: " << g.table() << std::endl;
    return 0;
}
