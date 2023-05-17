#include "morph/HdfData.h"
#include "morph/vvec.h"
#include "morph/vec.h"
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

    // Sadly a vvec of morph::vec is nok.
    morph::vvec<morph::vec<FLT, 3>> vvec_of_vec (vvv.size());
    vvec_of_vec[0] = morph::vec<FLT, 3>({1,2,3});
    vvec_of_vec[1] = morph::vec<FLT, 3>({2,2,3});
    vvec_of_vec[2] = morph::vec<FLT, 3>({3,2,3});
    vvec_of_vec[3] = morph::vec<FLT, 3>({4,2,3});

    // Check content
    for (auto vv : vvv) { std::cout << "vv: " << vv << std::endl; }

    {
        morph::HdfData data("test4.h5");
        data.add_contained_vals ("/vvv", vvv);
        data.add_contained_vals ("/vvec_of_vec", vvec_of_vec);
    } // data closes when out of scope

    // void read_contained_vals (const char* path, morph::vvec<morph::vvec<T>>& vals)
    morph::vvec<morph::vvec<FLT>> vvread;
    morph::vvec<morph::vec<FLT>> vvread_vvofv;
    {
        morph::HdfData data("test4.h5", morph::FileAccess::ReadOnly);
        data.read_contained_vals ("/vvv", vvread);
        data.read_contained_vals ("/vvec_of_vec", vvread_vvofv);
    }

    for (auto vv : vvread) {
        std::cout << "vv read: " << vv << std::endl;
    }

    if (vvv[0] == vvread[0]
        && vvv[3] == vvread[3]
        && vvec_of_vec[3][0] == vvread_vvofv[3][0]
        && vvec_of_vec[3][1] == vvread_vvofv[3][1]
        && vvec_of_vec[3][2] == vvread_vvofv[3][2]
        ) {
        rtn = 0;
    }

    return rtn;
}
