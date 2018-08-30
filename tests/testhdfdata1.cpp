#include "HdfData.h"
#include <utility>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
using morph::HdfData;

int main()
{
    int rtn = -1;

    vector<double> vd = { 1.0, 2.0, 3.0, 4.0 };
    {
        HdfData data("test.h5");
        data.add_double_vector ("/testvectordouble", vd);
    } // data closes when out of scope

    vector<double> vdread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_double_vector ("/testvectordouble", vdread);
    }

    if (vd.size() == vdread.size()) {
        rtn = 0;
        for (unsigned int i = 0; i < vd.size(); ++i) {
            if (vd[i] != vdread[i]) {
                rtn = -1;
                break;
            }
        }
    }

    return rtn;
}
