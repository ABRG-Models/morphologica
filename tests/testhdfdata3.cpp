#include "morph/HdfData.h"
#include <utility>
#include <iostream>
#include <fstream>
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

    cout << "vector<array<FLT, 3>>" << endl;
    vector<array<FLT, 3>> va3 = { { 1.0, 1.0, 1.0 },
                                 { 3.0, 2.0, 2.0 },
                                 { 5.0, 9.7, 2.0 },
                                 { 7.0, 8.1, 2.0 },
                                 { 9.0, 0.3, 0.3 } };
    {
        HdfData data("test3.h5");
        data.add_contained_vals ("/testvecarrayf3", va3);
    } // data closes when out of scope

    vector<array<FLT, 3>> varead3;
    {
        HdfData data("test3.h5", true); // true for read data
        data.read_contained_vals ("/testvecarrayf3", varead3);
    }

    if (va3.size() == varead3.size()) {
        for (unsigned int i = 0; i < va.size(); ++i) {
            if (va3[i][0] != varead3[i][0]) {
                rtn -= 1;
                break;
            }
            if (va3[i][1] != varead3[i][1]) {
                rtn -= 1;
                break;
            }
            cout << "Coordinate: (" << va3[i][0] << "," << va3[i][1] << "," << va3[i][2] << ")" << endl;
        }
    }

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
