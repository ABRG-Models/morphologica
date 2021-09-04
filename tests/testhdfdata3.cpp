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

    // Test vector of array<FLT,2>

    cout << "vector<array<FLT, 2>>" << endl;
    vector<array<FLT, 2>> va = { { 1.0, 1.0 },
                                 { 3.0, 2.0 },
                                 { 5.0, 9.7 },
                                 { 7.0, 8.1 },
                                 { 9.0, 0.3 } };
    {
        HdfData data("test3.h5");
        data.add_contained_vals ("/testvecarrayf2", va);
    } // data closes when out of scope

    vector<array<FLT, 2>> varead;
    {
        HdfData data("test3.h5", true); // true for read data
        data.read_contained_vals ("/testvecarrayf2", varead);
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
            cout << "Coordinate: (" << va[i][0] << "," << va[i][1] << ")" << endl;
        }
    }
#if 0
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
        cout << "vpd[0]: " << vpd[0] << endl;
    }
#endif

    // Save and retrieve a container of arrays
    {
        HdfData data("testvecarr.h5");
        std::deque<std::array<float,2>> vp;
        vp.push_back ({1,2});
        vp.push_back ({3,5});
        vp.push_back ({300,50});
        data.add_contained_vals ("/vecarrayfloat2", vp);
    }

    {
        HdfData data("testvecarr.h5", true);
        std::deque<std::array<float,2>> vpd;
        data.read_contained_vals("/vecarrayfloat2", vpd);
        cout << "vpd[0]: " << vpd[0][0] << "," << vpd[0][1] << endl;
    }

    cout << "Returning " << rtn << endl;

    return rtn;
}
