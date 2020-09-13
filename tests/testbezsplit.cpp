#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
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
    int rtn = 0;
    vector<pair<float,float>> c;
    pair<float,float> v1 = make_pair (1.0f,1.0f);
    pair<float,float> v2 = make_pair (2.0f,8.0f);
    pair<float,float> v3 = make_pair (9.0f,8.0f);
    pair<float,float> v4 = make_pair (10.0f,1.0f);
    c.push_back (v1);
    c.push_back (v2);
    c.push_back (v3);
    c.push_back (v4);

    BezCurve<FLT> cv (c);
    cout << "Defined a " << cv.getOrder() << " nd/rd/th order curve" << endl;

    cout << "cv = [" << cv.output(static_cast<FLT>(1.0)) << "];\n";

    pair<arma::Mat<FLT>, arma::Mat<FLT>> nc = cv.split (static_cast<FLT>(0.5));

    cout << "oc=[" << cv.outputControl() << "]\n";
    cout << "c1=[" << nc.first << "]\n";
    cout << "c2=[" << nc.second << "]\n";

    BezCurve<FLT> cv1 (nc.first);
    BezCurve<FLT> cv2 (nc.second);

    cout << "cv1 = [" << cv1.output(static_cast<FLT>(1.0)) << "];\n";
    cout << "cv2 = [" << cv2.output(static_cast<FLT>(1.0)) << "];\n";

    return rtn;
}
