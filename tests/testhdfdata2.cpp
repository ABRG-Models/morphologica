#include "morph/HdfData.h"
#include <opencv2/opencv.hpp>
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <list>
#include <vector>

using namespace std;
using morph::HdfData;

int main()
{
    int rtn = 0;

    // Test vector of vectors of floats
    // vector of OpenCL Points (& Point2ds)
    // vector of array<float,3>
    // save/retrieve strings

    cout << "vector<array<float, 3>>" << endl;
    vector<array<float, 3>> va = { { 1.0, 1.0, 2.0 },
                                   { 3.0, 3.0, 4.0 },
                                   { 5.0, 5.0, 6.0 },
                                   { 7.0, 7.0, 8.0 },
                                   { 9.0, 9.0, 10.0 } };
    {
        HdfData data("test0.h5");
        data.add_contained_vals ("/testvecarray", va);
    } // data closes when out of scope

    // Demonstrate appending data to an existing HDF5 file:
    {
        HdfData data("test0.h5", morph::FileAccess::ReadWrite);
        data.add_contained_vals ("/testvecarray2", va);
    }

    vector<array<float, 3>> varead;
    {
        HdfData data("test0.h5", morph::FileAccess::ReadOnly);
        data.read_contained_vals ("/testvecarray2", varead);
    }

    if (va.size() == varead.size()) {
        for (unsigned int i = 0; i < va.size(); ++i) {
            if (va[i][0] != varead[i][0]) {
                rtn -= 1;
                break;
            }
            if (va[i][1] != varead[i][1]) {
                rtn -= 1;
                break;
            }
            if (va[i][2] != varead[i][2]) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << va[i][0] << "," << va[i][1] << "," << va[i][2] << ")" << endl;
        }
    }

    // Demonstrate overwriting data to an existing HDF5 file:
    va[0][0] = 100.0f;
    {
        HdfData data("test0.h5", morph::FileAccess::ReadWrite);
        data.add_contained_vals ("/testvecarray2", va);
    }
    // And read back:
    {
        HdfData data("test0.h5", morph::FileAccess::ReadOnly);
        data.read_contained_vals ("/testvecarray2", varead);
    }
    cout << "varead[0][0] = " << varead[0][0] << " (should be 100) varead size: " << varead.size() << "\n";
    if (varead.size() != va.size()) { rtn -= 1; }
    if (varead[0][0] != 100.0f) { rtn -= 1; }

    cout << "vector<array<float, 12>>" << endl;
    vector<array<float, 12>> va12 = { { 1., 1., 2., 1., 1., 2., 1., 1., 2., 1., 1., 2. },
                                      { 3., 3., 4., 2., 1., 2., 3., 3., 4., 3., 3., 4. },
                                      { 5., 5., 6., 3., 1., 2., 3., 3., 4., 3., 3., 4.  },
                                      { 5., 5., 6., 4., 1., 2., 3., 3., 4., 3., 3., 4.  },
                                      { 7., 7., 8., 5., 1., 2., 3., 3., 4., 3., 3., 4.  },
                                      { 9., 9., 10., 6., 1., 2., 3., 3., 4., 3., 3., 4.  } };
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/testvecf12", va12);
    } // data closes when out of scope

    vector<array<float, 12>> va12read;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/testvecf12", va12read);
    }

    if (va12.size() == va12read.size()) {
        for (unsigned int i = 0; i < va12.size(); ++i) {
            for (unsigned int j = 0; j < 12; ++j) {
                if (va12[i][j] != va12read[i][j]) {
                    rtn -= 1;
                    break;
                }
            }
            cout << "Coordinate: (" << va12[i][0] << "," << va12[i][1] << "," << va12[i][2] << "," << va12[i][3] << "...)" << endl;
        }
    }

    cout << "vector<Point>" << endl;
    vector<cv::Point> vpi = { cv::Point(1,2),
                              cv::Point(3,4),
                              cv::Point(5,6),
                              cv::Point(7,8),
                              cv::Point(9,18)};
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/vpi", vpi);
    } // data closes when out of scope

    vector<cv::Point> vpiread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/vpi", vpiread);
    }

    if (vpi.size() == vpiread.size()) {
        for (unsigned int i = 0; i < vpi.size(); ++i) {
            if (vpi[i].x != vpiread[i].x) {
                rtn -= 1;
                break;
            }
            if (vpi[i].y != vpiread[i].y) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << vpiread[i].x << "," << vpiread[i].y << ")" << endl;
        }
    }

    cout << "vector<Point2f>" << endl;
    vector<cv::Point2f> vpi2f = { cv::Point2f(1.5,2.5),
                                  cv::Point2f(3.,4.),
                                  cv::Point2f(5.,6.7),
                                  cv::Point2f(7.6,8.0),
                                  cv::Point2f(9.2,18.3)};
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/vpi2f", vpi2f);
    } // data closes when out of scope

    vector<cv::Point2f> vpi2fread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/vpi2f", vpi2fread);
    }

    if (vpi2f.size() == vpi2fread.size()) {
        for (unsigned int i = 0; i < vpi2f.size(); ++i) {
            if (vpi2f[i].x != vpi2fread[i].x) {
                rtn -= 1;
                break;
            }
            if (vpi2f[i].y != vpi2fread[i].y) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << vpi2fread[i].x << "," << vpi2fread[i].y << ")" << endl;
        }
    }

    cout << "vector<Point2d>" << endl;
    vector<cv::Point2d> vpi2d = { cv::Point2d(1.5,2.5),
                                  cv::Point2d(3.,4.),
                                  cv::Point2d(5.,6.7),
                                  cv::Point2d(7.6,8.0),
                                  cv::Point2d(9.2,18.3)};
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/vpi2d", vpi2d);
    } // data closes when out of scope

    vector<cv::Point2d> vpi2dread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/vpi2d", vpi2dread);
    }

    if (vpi2d.size() == vpi2dread.size()) {
        for (unsigned int i = 0; i < vpi2d.size(); ++i) {
            if (vpi2d[i].x != vpi2dread[i].x) {
                rtn -= 1;
                break;
            }
            if (vpi2d[i].y != vpi2dread[i].y) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << vpi2dread[i].x << "," << vpi2dread[i].y << ")" << endl;
        }
    }

    cout << "vector<pair<ULL,ULL>>" << endl;
    vector<std::pair<unsigned long long int, unsigned long long int>> vpi2dpair = { std::make_pair(1ULL,3ULL),
                                                                                    std::make_pair(3ULL,4ULL),
                                                                                    std::make_pair(5ULL,7ULL),
                                                                                    std::make_pair(8ULL,8ULL),
                                                                                    std::make_pair(9ULL,18ULL) };
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/vpi2dpair", vpi2dpair);
    } // data closes when out of scope

    vector<std::pair<unsigned long long int, unsigned long long int>> vpi2dpairread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/vpi2dpair", vpi2dpairread);
    }

    if (vpi2dpair.size() == vpi2dpairread.size()) {
        //// To here
        for (unsigned int i = 0; i < vpi2dpair.size(); ++i) {
            if (vpi2dpair[i].first != vpi2dpairread[i].first) {
                rtn -= 1;
                break;
            }
            if (vpi2dpair[i].second != vpi2dpairread[i].second) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << vpi2dpairread[i].first << "," << vpi2dpairread[i].second << ")" << endl;
        }
    }

    string tstr = "Thou art more lovely...";
    {
        HdfData data("test.h5");
        data.add_string ("/stringtest", tstr);
    }
    string str;
    {
        HdfData data("test.h5", true);
        data.read_string ("/stringtest", str);
    }
    cout << "String stored: " << tstr << endl;
    cout << "String retrieved: " << str << endl;
    if (str != tstr) {
        rtn -= 1;
    }

    bitset<13> bs;
    bs.set(3);
    bs.set(7);
    {
        HdfData data("test.h5");
        data.add_val ("/bitset", bs);
    }
    bitset<13> bsread;
    {
        HdfData data("test.h5", true);
        data.read_val ("/bitset", bsread);
    }
    cout << "Bitset stored: " << bs << endl;
    cout << "Bitset retrieved: " << bsread << endl;
    if (!(bs == bsread)) {
        rtn -= 1;
    }

    cv::Mat mt = cv::Mat::ones(6, 7, CV_64F);
    mt.at<double>(2,3) = 2.f;
    mt.at<double>(5,5) = -7.5f;
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/matfloat", mt);
    }
    cout << "Mat stored: " << mt << endl;
    cv::Mat mtread;
    {
        HdfData data("test.h5", true);
        data.read_contained_vals ("/matfloat", mtread);
    }
    cout << "Mat retrieved: " << mtread << endl;

    //if (!(bs == bsread)) {
    //   rtn -= 1;
    //}

    cout << "Returning " << rtn << endl;

    return rtn;
}
