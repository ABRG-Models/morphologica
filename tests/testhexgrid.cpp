#include "HexGrid.h"
#include <iostream>

using namespace morph;
using namespace std;

int main()
{
    HexGrid hg(2.309401, 4000);
    cout << "Set up " << hg.num() << " hexes in a grid." << endl;
    if (hg.num() == 2257669) {
        return 0;
    }
    return -1;
}
