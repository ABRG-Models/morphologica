#include <morph/trait_tests.h>
#include <iostream>

template <typename _S=float>
std::enable_if_t < morph::is_copyable_container<_S>::value, bool >
set_from (const _S& v)
{
        std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is a simple, copyable container" << std::endl;
        return true;
}
template <typename _S=float>
std::enable_if_t < !morph::is_copyable_container<_S>::value, bool >
set_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " isn't a container" << std::endl;
    return false;
}

template <typename _S=float>
std::enable_if_t < morph::is_copyable_fixedsize<_S>::value, bool >
set_from_fixed (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is a fixed size, simple, copyable container" << std::endl;
    return true;
}
template <typename _S=float>
std::enable_if_t < !morph::is_copyable_fixedsize<_S>::value, bool >
set_from_fixed (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is NOT a fixed size, simple, copyable container" << std::endl;
    return false;
}

template <typename _S=float>
std::enable_if_t < !morph::is_complex<_S>::value, bool >
complex_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " isn't a complex" << std::endl;
    return false;
}
template <typename _S=float>
std::enable_if_t < morph::is_complex<_S>::value, bool >
complex_from (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is a complex" << std::endl;
    return true;
}

#include <morph/vec.h>
#include <morph/vvec.h>
#include <set>
#include <array>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <complex>

int main()
{
    int rtn = 0;

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

    std::complex<float> c5;
    bool complex_can = complex_from (c5);

    float c6;
    bool float_is_complex = complex_from (c6);

    if (float_can || !array_can || !vector_can || !set_can || !complex_can || float_is_complex) {
        std::cout << "Test failed\n";
        --rtn;
    }

    bool c_is_fixed = set_from_fixed (c);
    if (!c_is_fixed) { --rtn; }

    bool c2_is_fixed = set_from_fixed (c2);
    if (c2_is_fixed) { --rtn; }


    std::cout << "array is fixed size? " << (morph::is_copyable_fixedsize<std::array<float, 2>>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<std::array<float, 2>>::value == false) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::array<float, 2>>::value == false) { --rtn; }
    // if constexpr (morph::is_copyable_fixedsize<std::array<float, 2>&>::value == false) { --rtn; } // fails to compile

    std::cout << "morph::vec is fixed size? " << (morph::is_copyable_fixedsize<morph::vec<double, 56>>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<morph::vec<double, 56>>::value == false) { --rtn; }
    // if (morph::is_copyable_fixedsize<morph::vec<double, 56>>::value == false) { --rtn; } // fails to compile
    if constexpr (morph::is_copyable_fixedsize<morph::vec<double, 56>>::value == false) { --rtn; }

    std::cout << "vector is fixed size? " << (morph::is_copyable_fixedsize<std::vector<double>&>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<std::vector<double>>::value == true) { --rtn; }
    if (morph::is_copyable_fixedsize<std::vector<double>&>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::vector<double>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::vector<double>&>::value == true) { --rtn; }

    std::cout << "morph::vvec is fixed size? " << (morph::is_copyable_fixedsize<morph::vvec<unsigned char>>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<morph::vvec<unsigned char>>::value == true) { --rtn; }
    if (morph::is_copyable_fixedsize<morph::vvec<unsigned char>&>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<morph::vvec<unsigned char>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<morph::vvec<unsigned char>&>::value == true) { --rtn; }

    std::cout << "list is fixed size? " << (morph::is_copyable_fixedsize<std::list<double>>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<std::list<double>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::list<double>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::list<double>&>::value == true) { --rtn; }

    std::cout << "deque is fixed size? " << (morph::is_copyable_fixedsize<std::deque<double>>::value ? "true" : "false") << std::endl;
    if (morph::is_copyable_fixedsize<std::deque<double>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::deque<double>>::value == true) { --rtn; }
    if constexpr (morph::is_copyable_fixedsize<std::deque<double>&>::value == true) { --rtn; }

    std::cout << "Test " << (rtn == 0 ? " PASSED" : " FAILED") << std::endl;
    return rtn;
}
