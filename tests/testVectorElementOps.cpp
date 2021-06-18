#include "morph/Vector.h"
#include "morph/vVector.h"
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    morph::Vector<float, 4> v = {1,2,3};

    cout << "v: " << v << " v.sq(): " << v.sq() << endl;
    cout << "v: " << v << " v.log(): " << v.log() << endl;
    cout << "v: " << v << " v.exp(): " << v.exp() << endl;
    v = { -2, 2, 3, -10 };
    cout << "v: " << v << " v.abs(): " << v.abs() << endl;

    cout << "v: " << v;
    v.sq_inplace();
    cout << " v.sq (in place): " << v << endl;

    morph::vVector<float> vv = { 1, 2, 3, 4 };
    cout << "vv before rotate pairs: " << vv;
    vv.rotate_pairs();
    cout << " vv after rotate_pairs: " << vv << endl;

    morph::vVector<float> vv2 = { 2, 2, 3, 8 };
    cout << "vv=" << vv << ", vv2=" << vv2 << endl;
    cout << "vv/vv2 = " << (vv / vv2) << endl;

    cout << "vv*vv2 = " << (vv * vv2) << endl;

    v = { 1, 2, 3, 4 };
    cout << "v before rotate pairs: " << v;
    v.rotate_pairs();
    cout << " v after rotate_pairs: " << v << endl;

    morph::Vector<float, 4> v2 = { 2, 2, 3, 8 };
    cout << "v=" << v << ", v2=" << v2 << endl;
    cout << "v/v2 = " << (v / v2) << endl;

    cout << "v*v2 = " << (v * v2) << endl;

    return rtn;
}
