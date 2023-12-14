#include <morph/trait_tests.h>
#include <iostream>

template <typename _S=float>
std::enable_if_t < morph::container_with_legacy_input_iterator<_S>::value, bool >
set_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " CAN be a legacy input iterator" << std::endl;
    return true;
}

template <typename _S=float>
std::enable_if_t < !morph::container_with_legacy_input_iterator<_S>::value, bool >
set_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " can't be a legacy input iterator" << std::endl;
    return false;
}

#include <array>
#include <vector>
#include <map>
int main()
{
    float f = 0.0f;
    bool float_can = set_from (f);
    std::array<double, 10> c;
    bool array_can = set_from (c);
    std::vector<double> c2;
    bool vector_can = set_from (c2);
    // I want false returned for std::map as this can't be set_from. So it's not JUST that map has
    // to have a LegacyInputIterator, because you can't std::copy(map::iterator, map::iterator,
    // vector::iterator). So leaving this FAILING for now.
    std::map<int, double> c3;
    bool map_can = set_from (c3);

    if (float_can || !array_can || !vector_can || map_can) {
        return -1;
    }

    return 0;
}
