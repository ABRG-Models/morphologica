#include <morph/trait_tests.h>
#include <iostream>
#include <set>

#if 0
template<typename F=float>
bool set_from (const std::set<F>& st)
{
    std::cout << "Type set<F>=set<" << typeid(F).name() << "> size " << sizeof (st) << " is a set container" << std::endl;
    return true;
}
#endif
template <typename _S=float>
std::enable_if_t < morph::is_copyable_container<_S>::value, bool >
set_from (const _S& v)
{
#if 0
    if constexpr (morph::has_find_method<_S>::value == true) {
        std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " seems to be a map container: " << morph::has_find_method<_S>::value << std::endl;
        return false;
    } else {
#endif
        std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is a simple, copyable container" << std::endl;
        return true;
#if 0
    }
#endif
}

template <typename _S=float>
std::enable_if_t < !morph::is_copyable_container<_S>::value, bool >
set_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " isn't a container" << std::endl;
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
    //std::map<int, double> c3;
    //bool map_can = set_from (c3);

    std::set<double> c4;
    bool set_can = set_from (c4);

    if (float_can || !array_can || !vector_can || !set_can) {
        std::cout << "Test failed\n";
        return -1;
    }

    std::cout << "Test passed\n";
    return 0;
}
