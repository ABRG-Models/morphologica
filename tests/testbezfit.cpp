#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <limits>

#include <chrono>
using namespace std::chrono;

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = -1;

    morph::vvec<morph::vec<float, 2>> c = {
        {-0.28f, 0.0f},
        {0.28f, 0.0f},
        {0.28f, 0.45f},
        {-0.28f, 0.45f}
    };

    BezCurve<FLT> cv;
    cv.fit (c);
    cout << "Defined a " << cv.getOrder() << " nd/rd/th order curve" << endl;

    // Now get points and output
    cout << "f=[\n" << cv.output (static_cast<unsigned int>(40)) << "];\n\n";

    cout << "p=[\n";
    for (auto p : c) {
        cout << p << endl;
    }
    cout << "];\n\n";

    cout << "c=[\n";
    cout << cv.outputControl();
    cout << "];\n\n";

    rtn = 0;

    return rtn;
}
