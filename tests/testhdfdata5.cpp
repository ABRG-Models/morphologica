#include "morph/HdfData.h"
#include "morph/vvec.h"

// Showing use of HdfData with std::ios flags to specify file access

int main()
{
    int rtn = 0;

    morph::vvec<float> vv = { 1.0f, 2.0f, 3.0f };

    {
        morph::HdfData data("test5.h5", std::ios::out|std::ios::trunc);
        data.add_contained_vals ("/vv", vv);
    } // data closes when out of scope

    morph::vvec<float> vv1;
    {
        morph::HdfData data("test5.h5", std::ios::in);
        data.read_contained_vals ("/vv", vv1);
    } // data closes when out of scope

    if (vv != vv1) { --rtn; }

    return rtn;
}
