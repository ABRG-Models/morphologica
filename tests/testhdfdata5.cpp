/*
 * A test program that compiles with OpenCV and uses OpenCV-friendly features from
 * HdfData.h with the relevant definition.
 */

// To use HdfData with OpenCV datatypes, you have to define this:
#define BUILD_HDFDATA_WITH_OPENCV 1
#include "morph/HdfData.h"

#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <list>
#include <vector>
#include <deque>

using namespace std;
using morph::HdfData;

int main()
{
    int rtn = 0;

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

    cv::Mat mt = cv::Mat::ones(6, 7, CV_64F);
    mt.at<double>(2,3) = 2.f;
    mt.at<double>(5,5) = -7.5f;
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/matfloat", mt);
    }
    //cout << "Mat stored: " << mt << endl; // Why doesn't this work?
    cout << "Mat[0] stored: " << mt.at<double>(2,3) << endl; // Why doesn't this work?
    cv::Mat mtread;
    {
        HdfData data("test.h5", true);
        data.read_contained_vals ("/matfloat", mtread);
    }
    cout << "Mat[0] retrieved: " << mtread.at<double>(2,3) << endl;

    if (mt.at<double>(2,3) != mtread.at<double>(2,3) || mt.at<double>(5,5) != mtread.at<double>(5,5)) { rtn -= 1; }

    // Test the saving of cv::Points
    {
        HdfData data("test3.h5");

        cv::Point p;
        p.x = 1;
        p.y = 45;
        data.add_contained_vals ("/Point_i", p);

        cv::Point2d pd;
        pd.x = 7.6;
        pd.y = 4.5;
        data.add_contained_vals ("/Point_d", pd);

        cv::Point2f pf;
        pf.x = 1.1f;
        pf.y = 3.3f;
        data.add_contained_vals ("/Point_f", pf);
    }

    // Save and retrieve a container of cv::Points
    {
        HdfData data("testvecpoints.h5");
        std::deque<cv::Point2i> vp;
        vp.push_back (cv::Point2i(1,2));
        vp.push_back (cv::Point2i(3,5));
        vp.push_back (cv::Point2i(300,50));
        data.add_contained_vals ("/vecpoints_i", vp);
        std::deque<cv::Point2d> vpd;
        vpd.push_back (cv::Point2d(1,2));
        vpd.push_back (cv::Point2d(3,5));
        vpd.push_back (cv::Point2d(300,50));
        data.add_contained_vals ("/vecpoints_d", vpd);
        std::deque<cv::Point2f> vpf;
        vpf.push_back (cv::Point2f(1,2));
        vpf.push_back (cv::Point2f(3,5));
        vpf.push_back (cv::Point2f(300,50));
        data.add_contained_vals ("/vecpoints_f", vpf);
    }

    {
        HdfData data("testvecpoints.h5", true);
        std::deque<cv::Point2d> vpd;
        data.read_contained_vals("/vecpoints_d", vpd);
        //cout << "vpd[0]: " << vpd[0] << endl; // why can't I stream a Point to cout?
    }

    return rtn;
}
