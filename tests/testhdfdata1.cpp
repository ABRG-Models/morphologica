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

    deque<double> vd = { 1.0, 2.0, 3.0, 4.0 };
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/testvectordouble", vd);
    } // data closes when out of scope

    deque<double> vdread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/testvectordouble", vdread);
    }

    if (vd.size() == vdread.size()) {
        for (unsigned int i = 0; i < vd.size(); ++i) {
            if (vd[i] != vdread[i]) {
                rtn -= 1;
                break;
            }
        }
    }

    vector<float> vf = { 1.0, 2.0, 3.0, 4.0 };
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/testvectorfloat", vf);
    } // data closes when out of scope

    vector<float> vfread;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/testvectorfloat", vfread);
    }

    if (vf.size() == vfread.size()) {
        for (unsigned int i = 0; i < vf.size(); ++i) {
            if (vf[i] != vfread[i]) {
                rtn -= 1;
                break;
            }
        }
    }

    list<pair<double, double>> listofpairs;
    listofpairs.push_back (make_pair(1.0,2.3));
    listofpairs.push_back (make_pair(1.3,2.4));
    listofpairs.push_back (make_pair(1.5,2.6));
    listofpairs.push_back (make_pair(1.9,2.9));
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/testlistofpairs", listofpairs);
    }
    list<pair<double, double>> listofpairs_read;
    {
        HdfData data("test.h5", true); // true for read data
        data.read_contained_vals ("/testlistofpairs", listofpairs_read);
    }
    if (listofpairs.size() == listofpairs_read.size()) {
        auto li = listofpairs.begin();
        auto lird = listofpairs_read.begin();
        while (li != listofpairs.end() && lird != listofpairs_read.end()) {
            if (li->first == lird->first && li->second == lird->second) {
                // Good
            } else {
                rtn -= 1;
            }
            li++;
            lird++;
        }

    } else {
        rtn -= 1;
    }

    pair<float, float> pr = { 3.0f, 6.0f };
    {
        HdfData data("test.h5");
        data.add_contained_vals ("/pair", pr);
    }
    pair<float, float> pr_rd;
    {
        HdfData data("test.h5", true);
        data.read_contained_vals ("/pair", pr_rd);
    }
    if (pr_rd.first != pr_rd.first || pr_rd.second != pr_rd.second) {
        rtn -= 1;
    }

    if (rtn != 0) {
        cout << "Failed " << (-rtn) << " times" << endl;
    }

    return rtn;
}
