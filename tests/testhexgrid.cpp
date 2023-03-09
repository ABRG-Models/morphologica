#include "morph/HexGrid.h"
#include <iostream>

int main()
{
    morph::HexGrid hg(2.309401, 400, .0f);
    std::cout << "Set up " << hg.num() << " hexes in a grid.\n";
    return hg.num() == 22969 ? 0 : -1;
}
