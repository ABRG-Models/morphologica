#include "HexGrid.h"
#include "BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
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
    vector<pair<float,float>> c;
#if 1
    pair<float,float> v1 = make_pair (-0.28f, 0.0f);
    pair<float,float> v2 = make_pair (0.28f, 0.0f);
    pair<float,float> v3 = make_pair (0.28f, 0.45f);
    pair<float,float> v4 = make_pair (-0.28f, 0.45f);
    c.push_back (v1);
    c.push_back (v2);
    c.push_back (v3);
    c.push_back (v4);
#else
    pair<float,float> v1 = make_pair (9.0f,10.0f);
    pair<float,float> v2 = make_pair (29.0f,16.0f);
    pair<float,float> v3 = make_pair (42.0f,33.0f);
    pair<float,float> v4 = make_pair (56.0f,47.0f);
    pair<float,float> v5 = make_pair (75.0f,52.0f);
    pair<float,float> v6 = make_pair (94.0f,59.0f);
    pair<float,float> v7 = make_pair (110.0f,68.0f);
    c.push_back (v1);
    c.push_back (v2);
    c.push_back (v3);
    c.push_back (v4);
    c.push_back (v5);
    c.push_back (v6);
    c.push_back (v7);
#endif

    BezCurve<FLT> cv;
    cv.fit (c);
    cout << "Defined a " << cv.getOrder() << " nd/rd/th order curve" << endl;

    // Now get points and output
    cout << "f=[\n" << cv.output (static_cast<unsigned int>(40)) << "];\n\n";

    cout << "p=[\n";
    for (auto p : c) {
        cout << p.first << "," << p.second << endl;
    }
    cout << "];\n\n";

    cout << "c=[\n";
    cout << cv.outputControl();
    cout << "];\n\n";

    rtn = 0;

    return rtn;
}
