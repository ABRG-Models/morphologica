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

    cout << "Returning " << rtn << endl;

    return rtn;
}
