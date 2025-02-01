#include <morph/trait_tests.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <set>
#include <array>
#include <vector>
#include <list>
//#include <deque>


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

template <typename _S=float>
std::enable_if_t < morph::is_copyable_fixedsize<_S>::value, bool >
set_from_fixed (const _S& v)
{
    std::cout << "Type _S=" << typeid(_S).name() << " size " << sizeof (v) << " is a fixed size, simple, copyable container" << std::endl;
    return true;
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

#include <array>
#include <vector>
#include <map>
#include <complex>

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

    std::complex<float> c5;
    bool complex_can = complex_from (c5);

    float c6;
    bool float_is_complex = complex_from (c6);

    if (float_can || !array_can || !vector_can || !set_can || !complex_can || float_is_complex) {
        std::cout << "Test failed\n";
        return -1;
    }

    bool c_is_fixed = set_from_fixed (c);
    if (!c_is_fixed) { return -1; }


#if 0 // This won't compile as c2 does not have constexpr size(). Haven't figured out
      // how to make it compile, but return false when c2 is a non-array/vec type
    bool c2_is_fixed = set_from_fixed (c2);
    if (c2_is_fixed) { return -1; }
#endif

    std::cout << "array is fixed size? " << (morph::is_copyable_fixedsize<std::array<float, 2>>::value ? "true" : "false") << std::endl;
    std::cout << "morph::vec is fixed size? " << (morph::is_copyable_fixedsize<morph::vec<double, 56>>::value ? "true" : "false") << std::endl;
    std::cout << "vector is fixed size? " << (morph::is_copyable_fixedsize<std::vector<double>>::value ? "true" : "false") << std::endl;
    std::cout << "morph::vvec is fixed size? " << (morph::is_copyable_fixedsize<morph::vvec<unsigned char>>::value ? "true" : "false") << std::endl;
    // You can only apply is_copyable_fixedsize to types that can be constructed inside a constexpr function (list, for example, won't compile) with
    // error: the type 'const std::__cxx11::list<double>' of 'constexpr' variable 't' is not literal
    std::cout << "list is fixed size? " << (morph::is_copyable_fixedsize<std::list<double>>::value ? "true" : "false") << std::endl;
    // error is similar for deque:
    //std::cout << "deque is fixed size? " << (morph::is_copyable_fixedsize<std::deque<double>>::value ? "true" : "false") << std::endl;

#if 0
    template <typename T, std::enable_if_t<morph::is_constexpr_constructible<T>(0), int> = 0>
    constexpr bool test_constexpr_constructible() { return true; } // is constexpr
    template <typename T, std::enable_if_t<not morph::is_constexpr_constructible<T>(0), int> = 0>
    constexpr bool test_constexpr_constructible() { return false; } // not constexpr


    std::cout << "test array constexpr construct: " << morph::test_constexpr_constructible<std::array<int, 2>>() << std::endl;
    std::cout << "test vector constexpr construct: " << morph::test_constexpr_constructible<std::vector<int>>() << std::endl;
    std::cout << "test list constexpr construct: " << morph::test_constexpr_constructible<std::list<int>>() << std::endl;
#endif
    std::cout << "Test passed\n";
    return 0;
}
