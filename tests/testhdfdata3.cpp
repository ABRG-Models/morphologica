#include "HdfData.h"
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

    cout << "Returning " << rtn << endl;

    return rtn;
}
