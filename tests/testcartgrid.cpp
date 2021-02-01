#include <morph/CartGrid.h>
#include <iostream>

int main()
{
    morph::CartGrid cg(2.309401, 4000);
    std::cout << "Set up " << cg.num() << " pixels in a Cartesian grid." << std::endl;
    return (cg.num() == 2257669 ? 0 : -1);
}
