#include "HexGrid.h"
#include "BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = -1;

    pair<float,float> v1 = make_pair (-0.28f, 0.0f);
    pair<float,float> v2 = make_pair (0.28f, 0.0f);
    pair<float,float> v3 = make_pair (0.28f, 0.45f);
    pair<float,float> v4 = make_pair (-0.28f, 0.45f);

    morph::BezCurve<float> c1(v1,v2);
    morph::BezCurve<float> c2(v2,v3);
    morph::BezCurve<float> c3(v3,v4);
    morph::BezCurve<float> c4(v4,v1);
    cout << "instanciated curves" << endl;
    morph::BezCurvePath bound;
    cout << "instanciated curvepath" << endl;

    bound.addCurve(c1);
    bound.addCurve(c2);
    bound.addCurve(c3);
    bound.addCurve(c4);

    HexGrid* Hgrid = new HexGrid(0.02, 4.0, 0.0, morph::HexDomainShape::Boundary);
    cout << "setBoundary..." << endl;
    Hgrid->setBoundary (bound);
    cout << "Number of hexes is: " << Hgrid->num() << endl;

    if (Hgrid->num() == 783) {
        // Success
        rtn = 0;
    }

    return rtn;
}
