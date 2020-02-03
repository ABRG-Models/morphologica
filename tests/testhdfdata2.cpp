#include "HdfData.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>
#include <list>

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
        HdfData data("test.h5");
        data.add_contained_vals ("/testvecarray", va);
    } // data closes when out of scope

    vector<array<float, 3>> varead;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/testvecarray", varead);
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



    cout << "Returning " << rtn << endl;

    return rtn;
}
