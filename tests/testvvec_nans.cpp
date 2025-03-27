#include <limits>
#include <iostream>
#include <morph/vvec.h>

int main()
{
    int rtn = 0;

    morph::vvec<float> hasnans = { 1, 2, 3, 4, std::numeric_limits<float>::quiet_NaN(), 6, 7, 8, 9 };
    morph::vvec<float> nonans = { 1, 2, 3, 4, 6, 7, 8, 9 };

    std::cout << "mean of hasnans, NOT ignoring nans: " << hasnans.mean<false>() << std::endl;
    std::cout << "mean of hasnans, ignoring nans: " << hasnans.mean<true>() << std::endl;
    std::cout << "mean of nonans: " << nonans.mean() << std::endl;

    if (nonans.mean() != hasnans.mean<true>()) { --rtn; }

    std::cout << (rtn ? "FAIL" : "PASS") << std::endl;
    return rtn;
}
