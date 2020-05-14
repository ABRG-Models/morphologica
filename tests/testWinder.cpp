#include <opencv2/opencv.hpp>
#include <vector>
#include "Winder.h"
#include "BezCoord.h"
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
    std::cout << "0\n";
    morph::Winder w4(lvec);
    std::cout << "1\n";
    std::vector<float> px4 = {500,500};
    std::cout << "2, px4: " << px4[0] << "," << px4[1] << "\n";
    int wn4 = w4.wind (px4);
    cout << "Winding number = " << wn4 << endl;
    if (wn4 != 1) {
        --rtn;
    }

#if 0
    // Test with pair (has first and second attributes)
    std::cout << "0\n";
    std::vector<std::pair<double, double>> vpair;
    std::cout << "0.5\n";
    vpair.push_back (std::pair(0.0f,0.0));
    vpair.push_back (std::pair(1000.0f,0.0));
    vpair.push_back (std::pair(1000.0f,1000.0));
    vpair.push_back (std::pair(0.0f,1000.0));
    vpair.push_back (std::pair(0.0f,0.0));

    std::cout << "1\n";
    morph::Winder w5(vpair);
    std::cout << "2\n";
    int wn5 = w5.wind (std::pair(500.0f,500.0));
    std::cout << "3\n";
    cout << "Winding number = " << wn5 << endl;
    if (wn5 != 1) {
        --rtn;
    }
#endif
    return rtn;
}
