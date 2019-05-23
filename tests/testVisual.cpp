#include "Visual.h"
#include "HexGrid.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::Visual;
using morph::HexGrid;

int main()
{
    int rtn = -1;

    Visual v(800,600,"Test window");

    HexGrid hg(2.309401, 4000);
    cout << "Set up " << hg.num() << " hexes in a grid." << endl;

    vector<float> data;
    unsigned int nhex = hg.num();
    data.resize(nhex, 0.0);

    // Make some dummy data
    for (unsigned int hi=0; hi<nhex; ++hi) {
        data[hi] = 1.0 * hg.d_x[hi];
    }

    array<float, 3> offset = { 0.0, 0.0, 0.0 };
    unsigned int gridId = v.addHexGridVisual (&hg, data, offset);

    v.redraw();

    cout << "Enter key to end" << endl;

    int a;
    cin >> a;

    return rtn;
}
