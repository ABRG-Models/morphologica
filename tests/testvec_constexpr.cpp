/*
 * Test constexpr capable functions in vec.
 *
 * This file must be compiled with a C++20 compiler so that it is legal to have "a
 * definition of a variable for which no initialization is performed".
 *
 * Each constexpr function in morph::vec is tested within a global constexpr function.
 */

#include <morph/mathconst.h>
#include <morph/vec.h>

constexpr morph::vec<double, 3> vec_add()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> vce = v1 + v2;
    return vce;
}

constexpr morph::vec<double, 3> vec_div_scalar()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> vce = v1 / 2.0;
    return vce;
}

constexpr morph::vec<double, 3> vec_diveq_scalar()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    v1 /= 2.0;
    return v1;
}

constexpr morph::vec<double, 3> vec_div_vec()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 2.0, 2.0, 2.0 };
    morph::vec<double, 3> vce = v1 / v2;
    return vce;
}

constexpr morph::vec<double, 3> vec_diveq_vec()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 2.0, 2.0, 2.0 };
    v1 /= v2;
    return v1;
}

constexpr morph::vec<double, 3> vec_mult_vec()
{
    morph::vec<double, 3> v1 = { 4.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 5.0, 2.0, 5.0 };
    morph::vec<double, 3> vce = v1 * v2;
    return vce;
}

constexpr morph::vec<double, 3> vec_multeq_vec()
{
    morph::vec<double, 3> v1 = { 4.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 5.0, 2.0, 5.0 };
    v1 *= v2;
    return v1;
}

constexpr morph::vec<double, 3> vec_mult_scalar()
{
    morph::vec<double, 3> v1 = { 4.0, 2.0, 3.0 };
    double v2 = 5.0;
    morph::vec<double, 3> vce = v1 * v2;
    return vce;
}

constexpr morph::vec<double, 3> vec_multeq_scalar()
{
    morph::vec<double, 3> v1 = { 4.0, 2.0, 3.0 };
    double v2 = 5.0;
    v1 *= v2;
    return v1;
}

constexpr morph::vec<double, 3> vec_several_ops()
{
    morph::vec<double, 3> v1 = { 4.0, 2.0, 3.0 };
    morph::vec<double, 3> v2 = { 6.0, 5.0, 2.0 };
    morph::vec<double, 3> v3 = { 1.0, 1.0, 1.0 };
    v1 += v2;
    v2 = v1 - v3;
    v3 -= v2;
    v1 = (v3 + 1.0) - 2.0;
    v1 -= 0.5;
    v2 = v1 + 10.0;
    v2 += 10.0;
    v1 = 1.0 + v2;
    v2 = 1.0 - v1;
    v1 = 2.0 * v2;
    v2 = 4.0 / v1;
    return v1;
}

constexpr morph::vec<double, 3> vec_renormalize()
{
    morph::vec<double, 3> v1 = { 1.0, 2.0, 3.0 };
    v1.renormalize();
    return v1;
}

constexpr morph::vec<float, 3> vec_set_from1()
{
    std::array<float, 3> v1 = { 1.0f, 2.0f, 3.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

constexpr morph::vec<float, 3> vec_set_from2()
{
    std::array<float, 4> v1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

constexpr morph::vec<float, 3> vec_set_from3()
{
    std::array<float, 2> v1 = { 1.0f, 2.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

constexpr morph::vec<float, 3> vec_set_from4()
{
    morph::vec<float, 4> v1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

constexpr morph::vec<float, 3> vec_set_from5()
{
    morph::vec<float, 2> v1 = { 1.0f, 2.0f };
    morph::vec<float, 3> v2;
    v2.set_from (v1);
    return v2;
}

constexpr morph::vec<float, 3> vec_set_from6()
{
    float val = 4.4f;
    morph::vec<float, 3> v2;
    v2.set_from (val);
    return v2;
}

constexpr morph::vec<float, 100> vec_linspace()
{
    morph::vec<float, 100> v;
    v.linspace (0.0f, 99.0f);
    return v;
}

constexpr morph::vec<float, 100> vec_arange()
{
    morph::vec<float, 100> v;
    v.arange (0.0f, 100.0f, 1.0f);
    return v;
}

constexpr morph::vec<float, 100> vec_zero()
{
    morph::vec<float, 100> v;
    v.zero();
    return v;
}

constexpr morph::vec<float, 100> vec_setmax_setlowest()
{
    morph::vec<float, 100> v;
    v.set_max();
    v.set_lowest();
    return v;
}

constexpr morph::vec<double, 4> vec_dimchanges()
{
    morph::vec<unsigned int, 3> v00 = { 1, 2, 3 };
    morph::vec<int, 3> v0 = v00.as_int();
    morph::vec<double, 3> v = v0.as_uint().as_double() / 10.0;
    morph::vec<double, 4> v1 = v.plus_one_dim();
    morph::vec<double, 3> v2 = v1.less_one_dim();
    morph::vec<double, 4> v3 = v2.plus_one_dim (0.4);
    return v3;
}

constexpr morph::vec<double, 3> vec_floatchanges()
{
    morph::vec<double, 3> v = { 0.1, 0.2, 0.3 };
    morph::vec<float, 3> v1 = v.as_float();
    morph::vec<double, 3> v2 = v1.as_double();
    return v2;
}

constexpr morph::range<double> vec_range()
{
    morph::vec<double> v = {1, 2, 3};
    morph::range<double> r = v.range();
    return r;
}

constexpr morph::range<double> vec_rescale()
{
    morph::vec<double, 5> v = { 1, 2, 3, 4, 5 };
    v.rescale_neg();
    v.rescale_sym();
    v.rescale();
    return v.range();
}
constexpr morph::vec<double, 6> vec_rotate()
{
    morph::vec<double, 6> v = { 1, 2, 3, 4, 5, 6 };
    v.rotate();
    v.rotate(-2);
    v.rotate_pairs();
    return v;
}

constexpr double vec_stats()
{
    double rtn = 0.0;

    morph::vec<double, 5> v = { 1, 0, 0, 0, 0 };
    bool isunit = v.checkunit();

    if (isunit) {
        v = v.shorten (0.5);
        v = v.lengthen (0.5);
        rtn = v.length();
        rtn = v.length_sq();
        std::size_t idx = v.arglongest();
        if (idx == 0) {
            rtn = v.longest();
        } else {
            idx = v.argshortest();
            if (idx == 0) {
                rtn = 2.0;
            } else {
                rtn = 3.0;
            }
        }
        idx = v.argmax();
        if (idx == 0) {
            rtn = v.max();
        } else { rtn = 4.0; }
    }

    if (v.has_inf() || v.has_nan() || v.has_nan_or_inf()) {
        return 6.0;
    }

    v[3] = std::numeric_limits<double>::infinity();
    v[4] = std::numeric_limits<double>::quiet_NaN();
    v.replace_nan_with (0.0);
    v.replace_nan_or_inf_with (0.0);

    if (v.has_zero()) {
        return rtn;
    }

    return 5.0;
}

constexpr double vec_stats2()
{
    morph::vec<double, 5> v = { 1, 2, 6, 7, 9 };
    double _mean = v.mean();
    double _var = v.variance();
    double _std = v.std();
    double _sum = v.sum();
    double _prod = v.product();
    return _var + _mean + _std + _sum + _prod; // returns 800.891
}

constexpr morph::vec<double, 5> vec_ops()
{
    morph::vec<double, 5> p = { 4.0, 3.0, 2.0, 1.0, 1.0 };
    morph::vec<double, 5> v = { 1, 2, 6, 7, 9 };
    morph::vec<double, 5> v2 = v.pow (2.0);
    v2.pow_inplace (2.0);
    morph::vec<double, 5> v3 = v.pow (p);

    morph::vec<double, 5> v4 = v.signum();
    v4.signum_inplace();

    morph::vec<double, 5> vfl = { 1.1, 2.5, 5.7, 8.9, 1.0 };
    morph::vec<double, 5> v5 = vfl.floor();
    vfl.floor_inplace();

    morph::vec<double, 5> vce = { 1.1, 2.5, 5.7, 8.9, 1.0 };
    morph::vec<double, 5> v6 = vce.ceil();
    vce.ceil_inplace();

    morph::vec<double, 5> vtr = { 1.1, 2.5, 5.7, 8.9, 1.0 };
    morph::vec<double, 5> v7 = vtr.trunc();
    vtr.trunc_inplace();

    morph::vec<double, 5> vsqrt = { 25.0, 16.0, 9.0, 4.0, 1.0 };
    morph::vec<double, 5> v8 = vsqrt.sqrt().sq();
    vsqrt.sqrt_inplace();
    vsqrt.sq_inplace();

    return v3 + v5 - v3 + v6 + v7 + v8;
}

constexpr morph::vec<double, 5> vec_ops2()
{
    morph::vec<double, 5> v = { 1, 2, 3, 4, 5 };
    morph::vec<double, 5> v1 = v.log();
    morph::vec<double, 5> v2 = v1.exp();
    morph::vec<double, 5> v3 = v2.log10();

    v.log_inplace();
    v.exp_inplace();
    v.log10_inplace();

    morph::vec<double, 5> neg = { -1, -1, -1, -1, -1 };
    neg.abs_inplace();
    morph::vec<double, 5> neg1 = { -1, -1, -1, -1, -1 };

    return (v - v3 + neg + neg1.abs());
}

constexpr morph::vec<bool, 8> vec_comparisons()
{
    morph::vec<bool, 8> rtn = { false, false, false, false, false, false, false, false };

    morph::vec<double, 5> v1 = { 1, 2, 3, 4, 5 };
    morph::vec<double, 5> v2 = { 1.1, 2, 3, 4, 5 };
    morph::vec<double, 5> v3 = { 1, 2, 3, 4, 5 };

    rtn[0] = v1 < v2;
    rtn[1] = v1 <= v2;
    rtn[2] = v1 > v2;
    rtn[3] = v1 <= v3;
    rtn[4] = v1 < 6.0;
    rtn[5] = v1 <= 5.0;
    rtn[6] = v1 > 6.0;
    rtn[7] = v1 <= 4.0;

    return rtn;
}

constexpr bool vec_negate_not()
{
    morph::vec<double, 5> v = { 1, 2, 3, 4, 5 };
    morph::vec<double, 5> v2 = -v;
    bool notv2 = !v2;
    return notv2; // false
}

constexpr morph::vec<double, 3> vec_maths()
{
    morph::vec<double, 3> f1 = {-10, 0, 0};
    morph::vec<double, 3> f2 = {-20, 0, 0};
    morph::vec<double, 3> f3 = {-30, 0, 0};

    morph::vec<double, 3> x = { 1, 0, 0 };
    morph::vec<double, 3> z = { 0, 0, 1 };

    double dp = x.dot (z); // 0

    morph::vec<double, 2> xx = { 1, 0 };
    morph::vec<double, 2> yy = { 0, 1 };

    if (dp == 0.0) {
        morph::vec<double, 3> y = z.cross(x);
        double cp = xx.cross(yy);
        if (cp == 1.0) {
            return y;
        } else {
            return f3;
        }
    } else {
        return f2;
    }
    return f1;
}

constexpr morph::vec<float, 2> vec_angle()
{
    morph::vec<float, 2> va = { 0, 1 };
    float angle = va.angle();
    if (angle == morph::mathconst<float>::pi_over_2) {
        va.set_angle (morph::mathconst<float>::pi_over_4);
        return va;
    }
    return va;
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

    constexpr morph::vec<float, 3> result4 = vec_set_from1();
    if (result4[1] != 2.0f) { std::cout << "Fail 4\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result5 = vec_set_from2();
    if (result5[2] != 3.0f) { std::cout << "Fail 5\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result6 = vec_set_from3();
    if (result6[2] != 0.0f || result6[1] != 2.0f) { std::cout << "Fail 6\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result7 = vec_set_from4();
    if (result7[2] != 3.0f) { std::cout << "Fail 7\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result8 = vec_set_from5();
    if (result8[2] != 0.0f || result8[1] != 2.0f) { std::cout << "Fail 8\n"; rtn -= 1; }

    constexpr morph::vec<float, 3> result9 = vec_set_from6();
    if (result9[2] != 4.4f) { std::cout << "Fail 9\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result10 = vec_linspace();
    if (result10[0] != 0.0f || result10[99] != 99.0f) { std::cout << "Fail 10\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result11 = vec_arange();
    if (result11[0] != 0.0f || result11[99] != 99.0f) { std::cout << "Fail 11\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result12 = vec_zero();
    if (result12[1] != 0.0f || result12[67] != 0.0f) { std::cout << "Fail 12\n"; rtn -= 1; }

    constexpr morph::vec<float, 100> result13 = vec_setmax_setlowest();
    if (result13[1] != std::numeric_limits<float>::lowest()) { std::cout << "Fail 13\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result14 = vec_mult_vec();
    if (result14[0] != 20.0) { std::cout << "Fail 14\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result15 = vec_multeq_vec();
    if (result15[0] != 20.0) { std::cout << "Fail 15\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result16 = vec_mult_scalar();
    if (result16[0] != 20.0) { std::cout << "Fail 16\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result17 = vec_multeq_scalar();
    if (result17[0] != 20.0) { std::cout << "Fail 17\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result18 = vec_diveq_scalar();
    if (result18[0] != 0.5) { std::cout << "Fail 18\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result19 = vec_div_vec();
    if (result19[0] != 0.5) { std::cout << "Fail 19\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result20 = vec_diveq_vec();
    if (result20[0] != 0.5) { std::cout << "Fail 20\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result21 = vec_several_ops();
    if (result21[0] != -21.0) { std::cout << "Fail 21\n"; rtn -= 1; }

    constexpr morph::vec<double, 4> result22 = vec_dimchanges();
    if (result22[3] != 0.4) { std::cout << "Fail 22\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result23 = vec_floatchanges();
    if (std::abs(result23[0] - 0.1) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Fail 23" << result23 << "\n"; rtn -= 1;
    }

    constexpr morph::range<double> result24 = vec_range();
    if (result24.min != 1 || result24.max != 3) { std::cout << "Fail 24\n"; rtn -= 1; }

    constexpr morph::range<double> result25 = vec_rescale();
    if (result25.min != 0 || result25.max != 1) { std::cout << "Fail 25\n"; rtn -= 1; }

    constexpr morph::vec<double, 6> result26 = vec_rotate();
    if (result26[0] != 1.0 || result26[1] != 6.0) { std::cout << "Fail 26\n"; rtn -= 1; }

    constexpr double result27 = vec_stats();
    if (result27 != 1.0) { std::cout << "Fail 27, returned " << result27 << "\n"; rtn -= 1; }

    constexpr double result28 = vec_stats2();
    if (std::abs(result28 - 800.891) > 0.0005 ) { std::cout << "Fail 28\n"; rtn -= 1; }

    constexpr morph::vec<double, 5> result29 = vec_ops();
    if (result29[2] != 25.0 || result29[4] != 4.0) { std::cout << "Fail 29\n"; rtn -= 1; }

    constexpr morph::vec<double, 5> result30 = vec_ops2();
    if (result30[2] != 2.0 || result30[4] != 2.0) { std::cout << "Fail 30\n"; rtn -= 1; }

    constexpr morph::vec<bool, 8> result31 = vec_comparisons();
    if (result31[0] || !result31[1] || result31[2] || result31[7]) {
        std::cout << "Fail 31\n"; rtn -= 1;
    }

    constexpr bool result32 = vec_negate_not();
    if (result32 == true) { std::cout << "Fail 32\n"; rtn -= 1; }

    constexpr morph::vec<double, 3> result33 = vec_maths();
    if (result33[1] != 1) { std::cout << "Fail 33\n"; rtn -= 1; }

    constexpr morph::vec<float, 2> result34 = vec_angle();
    if (result34[0] != std::sqrt(2.0f)/2.0f) { std::cout << "Fail 34\n"; rtn -= 1; }

    return rtn;
}
