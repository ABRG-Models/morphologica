#include <iostream>
#include <vector>
#include <list>
#include "morph/Winder.h"
#include "morph/BezCoord.h"
#include "morph/vec.h"
#include "morph/vvec.h"

using std::cout;
using std::endl;
using std::array;

/*
 * Winder code should be able to compute the winding number of a coordinate with
 * respect to a container of coordinates which trace out a path. The coordinate used
 * for the Winder class could be std::array<float, 2> or std::vector<double>. It could
 * be pair<float, float> or BezCoord<float>. Let's test a few
 * possibilities. There's some function specialization in morph::Winder which makes
 * all this possible.
 */
int main() {

    int rtn = 0;

    // Test with morph::BezCoord, which has x() and y() methods
    std::vector<morph::BezCoord<float>> vbezc;
    vbezc.push_back (morph::BezCoord<float>(morph::vec<float,2>({0.0f,0.0f})));
    vbezc.push_back (morph::BezCoord<float>(morph::vec<float,2>({1000.0f,0.0f})));
    vbezc.push_back (morph::BezCoord<float>(morph::vec<float,2>({1000.0f,1000.0f})));
    vbezc.push_back (morph::BezCoord<float>(morph::vec<float,2>({0.0f,1000.0f})));
    vbezc.push_back (morph::BezCoord<float>(morph::vec<float,2>({0.0f,0.0f})));
    morph::Winder w2(vbezc);
    int wn2 = w2.wind (morph::BezCoord<float>(morph::vec<float,2>({500.0f,500.0f})));
    cout << "Winding number = " << wn2 << endl;
    if (wn2 != 1) {
        --rtn;
    }

    // Test with plain old std::array (and put this one in a list, too)
    std::list<std::array<float, 2>> larray;
    larray.push_back ({0.0f,0.0f});
    larray.push_back ({1000.0f,0.0f});
    larray.push_back ({1000.0f,1000.0f});
    larray.push_back ({0.0f,1000.0f});
    larray.push_back ({0.0f,0.0f});
    morph::Winder w3(larray);
    std::array<float, 2> px3 = {500.0f,500.0f};
    int wn3 = w3.wind (px3);
    cout << "Winding number = " << wn3 << endl;
    if (wn3 != 1) {
        --rtn;
    }

    // Test with std::vector
    std::list<std::vector<float>> lvec;
    lvec.push_back ({0.0f,0.0f});
    lvec.push_back ({1000.0f,0.0f});
    lvec.push_back ({1000.0f,1000.0f});
    lvec.push_back ({0.0f,1000.0f});
    lvec.push_back ({0.0f,0.0f});
    morph::Winder w4(lvec);
    std::vector<float> px4 = {500,500};
    int wn4 = w4.wind (px4);
    cout << "Winding number = " << wn4 << endl;
    if (wn4 != 1) {
        --rtn;
    }

    // Test with pair (has first and second attributes)
    std::vector<std::pair<double, double>> vpair;
    vpair.push_back (std::pair(0.0,0.0));
    vpair.push_back (std::pair(1000.0,0.0));
    vpair.push_back (std::pair(1000.0,1000.0));
    vpair.push_back (std::pair(0.0,1000.0));
    vpair.push_back (std::pair(0.0,0.0));
    morph::Winder w5(vpair);
    int wn5 = w5.wind (std::pair(500.0f,500.0));
    cout << "Winding number = " << wn5 << endl;
    if (wn5 != 1) {
        --rtn;
    }

    // morph::vvec
    std::list<morph::vvec<float>> lvVec;
    lvVec.push_back ({0.0f,0.0f});
    lvVec.push_back ({1000.0f,0.0f});
    lvVec.push_back ({1000.0f,1000.0f});
    lvVec.push_back ({0.0f,1000.0f});
    lvVec.push_back ({0.0f,0.0f});
    morph::Winder w6(lvVec);
    morph::vvec<float> px6 = {500.0f,500.0f};
    int wn6 = w6.wind (px6);
    cout << "Winding number = " << wn6 << endl;
    if (wn6 != 1) {
        --rtn;
    }

    // morph::vec
    std::vector<morph::vec<float, 2>> lVec;
    lVec.push_back ({0.0f,0.0f});
    lVec.push_back ({1000.0f,0.0f});
    lVec.push_back ({1000.0f,1000.0f});
    lVec.push_back ({0.0f,1000.0f});
    lVec.push_back ({0.0f,0.0f});
    morph::Winder w7(lVec);
    morph::vec<float, 2> px7 = {500.0f,500.0f};
    int wn7 = w7.wind (px7);
    cout << "Winding number = " << wn7 << endl;
    if (wn7 != 1) {
        --rtn;
    }

    return rtn;
}
