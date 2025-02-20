#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include "morph/vec.h"
#include <memory>
#include <iostream>
#include <fstream>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = -1;

    morph::vec<float, 2> v1 = {-0.28f, 0.0f};
    morph::vec<float, 2> v2 = {0.28f, 0.0f};
    morph::vec<float, 2> v3 = {0.28f, 0.45f};
    morph::vec<float, 2> v4 = {-0.28f, 0.45f};

    morph::BezCurve<float> c1(v1,v2);
    morph::BezCurve<float> c2(v2,v3);
    morph::BezCurve<float> c3(v3,v4);
    morph::BezCurve<float> c4(v4,v1);
    cout << "instanciated curves" << endl;
    morph::BezCurvePath<float> bound;
    cout << "instanciated curvepath" << endl;

    bound.addCurve(c1);
    bound.addCurve(c2);
    bound.addCurve(c3);
    bound.addCurve(c4);

    std::unique_ptr<HexGrid> Hgrid = std::make_unique<HexGrid>(0.02f, 4.0f, 0.0f);
    cout << "setBoundary..." << endl;
    Hgrid->setBoundary (bound);
    cout << "Number of hexes is: " << Hgrid->num() << endl;

    if (Hgrid->num() == 783) {
        // Success
        rtn = 0;
    }

    return rtn;
}
