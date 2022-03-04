#include "morph/HdfData.h"
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

    //if (!(bs == bsread)) {
    //   rtn -= 1;
    //}

    cout << "Returning " << rtn << endl;

    return rtn;
}
