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

    std::cout << "std of hasnans, NOT ignoring nans: " << hasnans.std<false>() << std::endl;
    std::cout << "std of hasnans, ignoring nans: " << hasnans.std<true>() << std::endl;
    std::cout << "std of nonans: " << nonans.std() << std::endl;
    if (nonans.std() != hasnans.std<true>()) { --rtn; }

    std::cout << "variance of hasnans, NOT ignoring nans: " << hasnans.variance<false>() << std::endl;
    std::cout << "variance of hasnans, ignoring nans: " << hasnans.variance<true>() << std::endl;
    std::cout << "variance of nonans: " << nonans.variance() << std::endl;
    if (nonans.variance() != hasnans.variance<true>()) { --rtn; }

    std::cout << "product of hasnans, NOT ignoring nans: " << hasnans.product<false>() << std::endl;
    std::cout << "product of hasnans, ignoring nans: " << hasnans.product<true>() << std::endl;
    std::cout << "product of nonans: " << nonans.product() << std::endl;
    if (nonans.product() != hasnans.product<true>()) { --rtn; }

    std::cout << "sum of hasnans, NOT ignoring nans: " << hasnans.sum<false>() << std::endl;
    std::cout << "sum of hasnans, ignoring nans: " << hasnans.sum<true>() << std::endl;
    std::cout << "sum of nonans: " << nonans.sum() << std::endl;
    if (nonans.sum() != hasnans.sum<true>()) { --rtn; }

    std::cout << "sos of hasnans, NOT ignoring nans: " << hasnans.sos<false>() << std::endl;
    std::cout << "sos of hasnans, ignoring nans: " << hasnans.sos<true>() << std::endl;
    std::cout << "sos of nonans: " << nonans.sos() << std::endl;
    if (nonans.sos() != hasnans.sos<true>()) { --rtn; }

    std::cout << "range of hasnans, NOT ignoring nans: " << hasnans.range<false>() << std::endl;
    std::cout << "range of hasnans, ignoring nans: " << hasnans.range<true>() << std::endl;
    std::cout << "range of nonans: " << nonans.range() << std::endl;
    if (nonans.range() != hasnans.range<true>()) { --rtn; }

    std::cout << (rtn ? "FAIL" : "PASS") << std::endl;
    return rtn;
}
