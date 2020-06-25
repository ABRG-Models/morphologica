#include <opencv2/opencv.hpp>
#include <vector>
#include <list>
#include "Winder.h"
#include "BezCoord.h"
#include "Vector.h"
#include "morph/vVector.h"

using std::cout;
using std::endl;
using std::array;

/*
 * Winder code should be able to compute the winding number of a coordinate with
 * respect to a container of coordinates which trace out a path. The coordinate used
 * for the Winder class could be std::array<float, 2> or std::vector<double>. It could
 * be pair<float, float>, BezCoord<float> or cv::Point. Let's test a few
 * possibilities. There's some function specialization in morph::Winder which makes
 * all this possible.
 */
int main() {

    int rtn = 0;

    // Test with cv::Point which has x and y attributes
    std::vector<cv::Point> vpoints;
    vpoints.push_back (cv::Point(0,0));
    vpoints.push_back (cv::Point(1000,0));
    vpoints.push_back (cv::Point(1000,1000));
    vpoints.push_back (cv::Point(0,1000));
    vpoints.push_back (cv::Point(0,0));
    morph::Winder w1(vpoints);
    int wn1 = w1.wind (cv::Point(500,500));
    cout << "Winding number = " << wn1 << endl;
    if (wn1 != 1) {
        --rtn;
    }

    // Test with morph::BezCoord, which has x() and y() methods
    std::vector<morph::BezCoord<float>> vbezc;
    vbezc.push_back (morph::BezCoord<float>(std::pair(0,0)));
    vbezc.push_back (morph::BezCoord<float>(std::pair(1000,0)));
    vbezc.push_back (morph::BezCoord<float>(std::pair(1000,1000)));
    vbezc.push_back (morph::BezCoord<float>(std::pair(0,1000)));
    vbezc.push_back (morph::BezCoord<float>(std::pair(0,0)));
    morph::Winder w2(vbezc);
    int wn2 = w2.wind (morph::BezCoord<float>(std::pair(500,500)));
    cout << "Winding number = " << wn2 << endl;
    if (wn2 != 1) {
        --rtn;
    }

    // Test with plain old std::array (and put this one in a list, too)
    std::list<std::array<float, 2>> larray;
    larray.push_back ({0,0});
    larray.push_back ({1000,0});
    larray.push_back ({1000,1000});
    larray.push_back ({0,1000});
    larray.push_back ({0,0});
    morph::Winder w3(larray);
    std::array<float, 2> px3 = {500,500};
    int wn3 = w3.wind (px3);
    cout << "Winding number = " << wn3 << endl;
    if (wn3 != 1) {
        --rtn;
    }

    // Test with std::vector
    std::list<std::vector<float>> lvec;
    lvec.push_back ({0,0});
    lvec.push_back ({1000,0});
    lvec.push_back ({1000,1000});
    lvec.push_back ({0,1000});
    lvec.push_back ({0,0});
    morph::Winder w4(lvec);
    std::vector<float> px4 = {500,500};
    int wn4 = w4.wind (px4);
    cout << "Winding number = " << wn4 << endl;
    if (wn4 != 1) {
        --rtn;
    }

    // Test with pair (has first and second attributes)
    std::vector<std::pair<double, double>> vpair;
    vpair.push_back (std::pair(0.0f,0.0));
    vpair.push_back (std::pair(1000.0f,0.0));
    vpair.push_back (std::pair(1000.0f,1000.0));
    vpair.push_back (std::pair(0.0f,1000.0));
    vpair.push_back (std::pair(0.0f,0.0));
    morph::Winder w5(vpair);
    int wn5 = w5.wind (std::pair(500.0f,500.0));
    cout << "Winding number = " << wn5 << endl;
    if (wn5 != 1) {
        --rtn;
    }

    // morph::vVector
    std::list<morph::vVector<float>> lvVec;
    lvVec.push_back ({0,0});
    lvVec.push_back ({1000,0});
    lvVec.push_back ({1000,1000});
    lvVec.push_back ({0,1000});
    lvVec.push_back ({0,0});
    morph::Winder w6(lvVec);
    morph::vVector<float> px6 = {500,500};
    int wn6 = w6.wind (px6);
    cout << "Winding number = " << wn6 << endl;
    if (wn6 != 1) {
        --rtn;
    }

    // morph::Vector
    std::vector<morph::Vector<float, 2>> lVec;
    lVec.push_back ({0,0});
    lVec.push_back ({1000,0});
    lVec.push_back ({1000,1000});
    lVec.push_back ({0,1000});
    lVec.push_back ({0,0});
    morph::Winder w7(lVec);
    morph::Vector<float, 2> px7 = {500,500};
    int wn7 = w7.wind (px7);
    cout << "Winding number = " << wn7 << endl;
    if (wn7 != 1) {
        --rtn;
    }

    return rtn;
}
