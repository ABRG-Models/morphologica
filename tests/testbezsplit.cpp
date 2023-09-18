#include "morph/HexGrid.h"
#include "morph/BezCurve.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>

using namespace std;
using morph::BezCoord;
using morph::BezCurve;
using morph::HexGrid;

int main()
{
    int rtn = 0;
    morph::vvec<morph::vec<float, 2>> c = {
        {1, 1},
        {2, 8},
        {9, 8},
        {10,1}
    };

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
