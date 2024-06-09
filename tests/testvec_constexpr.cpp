/*
 * Test constexpr capable functions in vec.
 *
 * This file must be compiled with a C++20 compiler so that it is legal to have "a
 * definition of a variable for which no initialization is performed".
 */

#include <morph/vec.h>

static constexpr morph::vec<double, 3> vec_add()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> vce = v1 + v2;
    return vce;
}

static constexpr morph::vec<double, 3> vec_div_scalar()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> vce = v1 / 2.0;
    return vce;
}

static constexpr morph::vec<double, 3> vec_renormalize()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    v1.renormalize();
    return v1;
}

int main()
{
    int rtn = 0;

    constexpr morph::vec<double, 3> result1 = vec_add();
    if (result1[0] != 2.0) { std::cout << "Fail 1\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result2 = vec_div_scalar();
    if (result2[0] != 1.0/2.0) { std::cout << "Fail 2\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result3 = vec_renormalize();
    if (result3.length() != 1.0) { std::cout << "Fail 3\n"; rtn -= 1; }

    return rtn;
}
