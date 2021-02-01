#include <morph/CartGrid.h>
#include <iostream>

int main()
{
    morph::CartGrid cg(2.0f, 8.0f);
    std::cout << "Set up " << cg.num() << " pixels in a Cartesian grid." << std::endl;

    for (auto a : cg.rects) {
        std::cout << a.outputCart() << std::endl;
    }

    return (cg.num() == 25 ? 0 : -1);
}
