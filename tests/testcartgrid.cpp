#include <morph/CartGrid.h>
#include <iostream>

int main()
{
    // A symmetric, zero-centered CartGrid
    morph::CartGrid cg(2.0f, 8.0f);
    cg.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg.num() << " pixels in a Cartesian grid of width/depth: " << cg.width() << "/" << cg.depth() << std::endl;

    for (auto a : cg.rects) {
        std::cout << a.outputCart() << std::endl;
    }

    // A CartGrid with 400 elements
    morph::CartGrid cg2(0.05f, 0.05f, 0.0f, 0.0f, 0.95f, 0.95f);
    cg2.setBoundaryOnOuterEdge();
    std::cout << "Set up " << cg2.num() << " pixels in a Cartesian grid of width/depth: " << cg2.width() << "/" << cg2.depth() << std::endl;
    for (auto a : cg2.rects) {
        std::cout << a.outputCart() << std::endl;
    }

    return (cg.num() == 25 ? 0 : -1);
}
