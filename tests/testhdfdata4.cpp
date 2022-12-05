#include "morph/HdfData.h"
#include "morph/vvec.h"
#include <iostream>

// Test containers of vvecs

int main()
{
    int rtn = 1;

    morph::vvec<morph::vvec<FLT>> vvv;
    vvv.push_back (morph::vvec<FLT>({1,2,3}));
    vvv.push_back (morph::vvec<FLT>({2,2,3}));
    vvv.push_back (morph::vvec<FLT>({3,2,3}));
    vvv.push_back (morph::vvec<FLT>({4,2,3}));

    // Check content
    for (auto vv : vvv) { std::cout << "vv: " << vv << std::endl; }

    {
        morph::HdfData data("test4.h5");
        data.add_contained_vals ("/vvv", vvv);
    } // data closes when out of scope

    // void read_contained_vals (const char* path, morph::vvec<morph::vvec<T>>& vals)
    morph::vvec<morph::vvec<FLT>> vvread;
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
