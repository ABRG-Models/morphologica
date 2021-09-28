#include "morph/HdfData.h"
#include "morph/vVector.h"
#include <iostream>

// Test containers of vVectors

int main()
{
    int rtn = 1;

    morph::vVector<morph::vVector<FLT>> vvv;
    vvv.push_back (morph::vVector<FLT>({1,2,3}));
    vvv.push_back (morph::vVector<FLT>({2,2,3}));
    vvv.push_back (morph::vVector<FLT>({3,2,3}));
    vvv.push_back (morph::vVector<FLT>({4,2,3}));

    // Check content
    for (auto vv : vvv) { std::cout << "vv: " << vv << std::endl; }

    {
        morph::HdfData data("test4.h5");
        data.add_contained_vals ("/vvv", vvv);
    } // data closes when out of scope

    // void read_contained_vals (const char* path, morph::vVector<morph::vVector<T>>& vals)
    morph::vVector<morph::vVector<FLT>> vvread;
    {
        morph::HdfData data("test4.h5", morph::FileAccess::ReadOnly);
        data.read_contained_vals ("/vvv", vvread);
    }

    for (auto vv : vvread) {
        std::cout << "vv read: " << vv << std::endl;
    }

    if (vvv[0] == vvread[0] && vvv[3] == vvread[3]) {
        rtn = 0;
    }
    return rtn;
}
