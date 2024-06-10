/*
 * Test constexpr capable functions in vec.
 *
 * This file must be compiled with a C++20 compiler so that it is legal to have "a
 * definition of a variable for which no initialization is performed".
 *
 * Each constexpr function in morph::vec is tested within a global constexpr function.
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

static constexpr morph::vec<float, 3> vec_set_from1()
{
    std::array<float, 3> v1 = { 1.0f, 2.0f, 3.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

static constexpr morph::vec<float, 3> vec_set_from2()
{
    std::array<float, 4> v1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

static constexpr morph::vec<float, 3> vec_set_from3()
{
    std::array<float, 2> v1 = { 1.0f, 2.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

static constexpr morph::vec<float, 3> vec_set_from4()
{
    morph::vec<float, 4> v1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

static constexpr morph::vec<float, 3> vec_set_from5()
{
    morph::vec<float, 2> v1 = { 1.0f, 2.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

static constexpr morph::vec<float, 3> vec_set_from6()
{
    float val = 4.4f;
    morph::vec<float, 3> v2;
    v2.set_from (val);
    return v2;
}

static constexpr morph::vec<float, 100> vec_linspace()
{
    morph::vec<float, 100> v;
    v.linspace (0.0f, 99.0f);
    return v;
}

static constexpr morph::vec<float, 100> vec_arange()
{
    morph::vec<float, 100> v;
    v.arange (0.0f, 99.0f, 1.0f);
    return v;
}

static constexpr morph::vec<float, 100> vec_zero()
{
    morph::vec<float, 100> v;
    v.zero();
    return v;
}

static constexpr morph::vec<float, 100> vec_setmax_setlowest()
{
    morph::vec<float, 100> v;
    v.set_max();
    v.set_lowest();
    return v;
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

    // set_from() overloads
    constexpr morph::vec<float, 3> result4 = vec_set_from1();
    if (result4[1] != 2.0f) { std::cout << "Fail 4\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result5 = vec_set_from2();
    if (result5[2] != 3.0f) { std::cout << "Fail 5\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result6 = vec_set_from3();
    if (result6[2] != 0.0f && result6[1] != 2.0f) { std::cout << "Fail 6\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result7 = vec_set_from4();
    if (result7[2] != 3.0f) { std::cout << "Fail 7\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result8 = vec_set_from5();
    if (result8[2] != 0.0f && result8[1] != 2.0f) { std::cout << "Fail 8\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result9 = vec_set_from6();
    if (result9[2] != 4.4f) { std::cout << "Fail 9\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result10 = vec_linspace();
    if (result10[0] != 0.0f && result10[99] != 99.0f) { std::cout << "Fail 10\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result11 = vec_arange();
    if (result11[0] != 0.0f && result11[99] != 99.0f) { std::cout << "Fail 11\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result12 = vec_zero();
    if (result12[1] != 0.0f && result12[67] != 0.0f) { std::cout << "Fail 12\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result13 = vec_setmax_setlowest();
    if (result13[1] != std::numeric_limits<float>::lowest()) { std::cout << "Fail 13\n"; rtn -= 1; }

    return rtn;
}
