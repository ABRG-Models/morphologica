#include "morph/vec.h"
#include "morph/mathconst.h"
#include <set>
#include <algorithm>
#include <complex>

using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;

    using namespace std::complex_literals;
    morph::vec<std::complex<double>, 4> cplx;
    cplx.set_from (std::pow(1i, 2));
    std::cout << "Complex*2: " << cplx*2.0 << std::endl;

    morph::vec<float, 4> v = {1,2,3};
    // Test x(), y() etc members
    cout << "x: " << v.x() << endl;
    cout << "z: " << v.z() << endl;
    cout << "w: " << v.w() << endl;
    // Test renormalize
    v.renormalize();
    cout << "After renormalize: " << v << endl;
    // Test checkunit
    cout << "is it unit? " << v.checkunit() << endl;
    // Test randomize
    v.randomize(3,4);
    cout << "After randomize: " << v << endl;
    // Check ints are ok, too
    morph::vec<int, 2> vi;
    vi.randomize(0,200);
    cout << "After randomize of int morph::vec: " << vi << endl;
    cout << "Length: " << vi.length() << endl;
    morph::vec<float, 3> vfl;
    vfl.randomize();
    cout << "Length of a float vector: " << vfl.length() << endl;
    // Test assignment
    morph::vec<int, 2> vi2 = vi;
    cout << "Copy of int morph::vec: " << vi2 << endl;
    // Test comparison
    cout << "vi == vi2? " << (vi == vi2 ? "yes" : "no") << endl;
    // Test negate
    morph::vec<int, 2> vi3 = -vi;
    cout << "-ve Copy of int morph::vec: " << vi3 << endl;
    // Test comparison
    cout << "vi == vi3? " << (vi == vi3 ? "yes" : "no") << endl;
    // Test cross product (3D only
    morph::vec<double, 3> a = {1,0,0};
    morph::vec<double, 3> b = {0,1,0};
    morph::vec<double, 3> c = a.cross(b);
    cout << a << " cross " << b << "=" << c << endl;
    // Test dot product
    morph::vec<int, 2> vv1 = {1,1};
    morph::vec<int, 2> vv2 = {2,2};
    int dp = vv1.dot (vv2);
    cout << vv1 << "." << vv2 << " = " << dp << endl;

    // Assignment
#if 0 //  Copy FROM std::array to vec is NO good
    // Test init from array
    array<float, 3> arr = { 2,3,4 };
    morph::vec<float, 3> varr = arr; // Tried overloading operator= to no avail.
    cout << "morph::vec from array: " << varr << endl;
#endif

#if 1 // copy FROM vec TO std::array is OK
    // Test init from array
    morph::vec<float, 3> varr = { 2, 3, 4 };
    std::array<float, 3> arr = varr; // copy TO std::array
    cout << "std::array assigned to from morph::vec: ";
    for (auto a : arr) { cout << a << ", "; }
    cout << std::endl;
#endif

    // Test scalar multiply
    vv2 *= 2UL;
    cout << "vv2 after *2:" << vv2 << endl;
    morph::vec<int, 2> vv4 = vv1 * 98;
    cout << "vv1 * 98:" << vv4 << endl;
    // Scalar division
    morph::vec<double, 3> d = a/3.0;
    cout << "a/3.0:" << d << endl;
    // morph::vec addition
    morph::vec<double, 3> e = a+b;
    cout << "a+b:" << e << endl;
    // morph::vec subtraction
    morph::vec<double, 3> f = a-b;
    cout << "a-b:" << f << endl;
    // Test default template args
    morph::vec<double> vd_def;
    vd_def.randomize();
    cout << vd_def << endl;
    morph::vec<> v_def;
    v_def.randomize();
    cout << v_def << endl;

    // So you want to do the dot product of a 1000000 D morph::vec? Easy
    morph::vec<float, 1000> big1;
    morph::vec<float, 1000> big2;
    big1.randomize(0,100);
    big2.randomize(0,20);
    cout << "DP..." << endl;
    float bdp = big1.dot(big2);
    cout << "big1.big2=" << bdp << endl;

    // Test setFrom
    morph::vec<double, 3> d1;
    array<double, 3> a1 = { 5,6,7 };
    d1.set_from (a1);
    cout << "d1 should be 5,6,7: " << d1 << endl;
    array<double, 4> a2 = { 5,6,8,8 };
    d1.set_from (a2);
    cout << "d1 should be 5,6,8: " << d1 << endl;
    d1.set_from (5.6);
    cout << "d1 should be 5.6 for all elements: " << d1 << endl;

    // Test hadamard operator* (elementwise multiplication)
    morph::vec<double, 3> h1 = {1.0, 2.0, 3.0};
    morph::vec<double, 3> h2 = {7.0, 6.0, 5.0};
    morph::vec<double, 3> h3 = h1 * h2;
    cout << h1 << "(o)" << h2 << " = " << h3 << endl;

    h1 *= h2;
    cout << "After h1 *= h2, h1: " << h1 << endl;

    // Test operator *= with different types. Ok if lhs is same type as result.
    morph::vec<int, 3> h4 = {2, 2, 2};
    //morph::vec<int, 3> h5 = h2 * h4; // Not ok
    morph::vec<int, 3> h6 = h4 * h2; // morph::vec<int, N> * morph::vec<double, N> implies expected loss of precision.
    morph::vec<double, 3> h7 = h2 * h4;
    //morph::vec<double, 3> h8 = h4 * h2; // Not ok
    cout << h2 << "(o)" << h4 << " = " << h6 << " or " << h7 << endl;

    // Will VS be happy?
    h4 *= h2;

    morph::vec<float, 4> maxlongest = {-1.1f, -7.0f, 3.0f, 6.0f };
    cout << "For morph::vec " << maxlongest
         << ", max: " << maxlongest.max() << " (at index "<< maxlongest.argmax()
         << "), longest component: " << maxlongest.longest() << " (at index "
         << maxlongest.arglongest() << ")\n";

    morph::vec<float, 4> totimes = { 1.0f, 2.0f, 3.0f, 4.0f };
    cout << "Cumulative product of " << totimes << " is " << totimes.product() << endl;

    morph::vec<float, 4> fordivision = { 1.0f, 2.0f, 3.0f, 4.0f };
    morph::vec<float, 4> divresult = 1.0f / fordivision;
    std::cout << 1.0f << " / " << fordivision << " = " << divresult << std::endl;

    morph::vec<float, 3> compare1 = { 1, 2, 3 };
    morph::vec<float, 3> compare2 = { 2, 1, 3 };

    std::cout << "compare1 < compare2: " << (compare1 < compare2) << std::endl;
    std::cout << "compare2 < compare1: " << (compare2 < compare1) << std::endl;

    auto _cmp = [](morph::vec<float, 3> a, morph::vec<float, 3> b) { return a.lexical_lessthan(b); };
    std::set<morph::vec<float, 3>, decltype(_cmp)> aset(_cmp);
    aset.insert (compare1);
    aset.insert (compare2);
    std::cout << "aset size " << aset.size() << std::endl;

    morph::vec<double,2> VV1 = {1.0, 2.0};
    morph::vec<double,2> VV2 = {2.0, 3.0};
    morph::vec<double,2> VV3 = {1.0, 30.0};
    morph::vec<morph::vec<double, 2>, 3> VdV = { VV1, VV2, VV3 };
    std::cout<< "VdV.mean() = " << VdV.mean() << std::endl;

    morph::vec<float, 2> v_continuous = { 0.5f, 0.6f };
    morph::vec<size_t , 2> v_discrete = { 1, 2 };
    morph::vec<float, 2> v_cd = v_continuous * v_discrete;
    std::cout << "You can do morph::vec<floattype,N> = morph::vec<floattype,N> * morph::vec<inttype,N>: " << v_cd << std::endl;
    // Can't do this though:
    //  morph::vec<float, 2> v_dc = v_discrete * v_continuous; // will throw compiler error

    morph::vec<int, 4> vr = { 0, 1, 2, 3 };
    morph::vec<int, 4> vr2 = vr;

    morph::vec<int, 7> rot_size_t_correct = { 0, 1, 2, 3, 0, 1, 2 };
    for (size_t i = 0; i < 7; ++i) {
        vr2 = vr;
        vr2.rotate (i);
        std::cout << vr << " rotate("<<i<<") is " << vr2 << std::endl;
        if (vr2[0] != rot_size_t_correct[i]) { --rtn; }
    }

    morph::vec<int, 14> rot_int_correct = { 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2 };
    for (int i = -7; i < 7; ++i) {
        vr2 = vr;
        vr2.rotate (i);
        std::cout << vr << " rotate("<<i<<") is " << vr2 << std::endl;
        if (vr2[0] != rot_int_correct[i+7]) { --rtn; }
    }

    morph::vec<float, 3> formax;
    formax.set_max();
    std::cout << "vec<float, 3>::set_max gives: " << formax << std::endl;
    formax.set_lowest();
    std::cout << "vec<float, 3>::set_lowest gives: " << formax << std::endl;

    // Test shorten
    morph::vec<float, 2> lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    morph::vec<float, 2> sv = lv.shorten (5.0f);
    std::cout << "lv: " << lv << " lv.shorten(5.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 3.0f, 4.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (10.0f);
    std::cout << "lv: " << lv << " lv.shorten(10.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (12.0f);
    std::cout << "lv: " << lv << " lv.shorten(12.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (-5.0f); // shorten -ve lengthens
    std::cout << "lv: " << lv << " lv.shorten(-5.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 9.0f, 12.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-5.0f); // lengthen -ve shortens
    std::cout << "lv: " << lv << " lv.lengthen(-5.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 3.0f, 4.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-10.0f);
    std::cout << "lv: " << lv << " lv.lengthen(-10.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-12.0f);
    std::cout << "lv: " << lv << " lv.lengthen(-12.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (5.0f);
    std::cout << "lv: " << lv << " lv.lengthen(5.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 9.0f, 12.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (15.0f);
    std::cout << "lv: " << lv << " lv.lengthen(15.0f) returns the vector: " << sv << std::endl;
    if (sv != morph::vec<float, 2>({ 15.0f, 20.0f })) { --rtn; }

    morph::vec<float> fv1 = { 0.0f,0.0f,0.0f };
    float vmult = 0.045f;
    std::cout << "Before multiply 0 vector by " << vmult << " it is " << fv1;
    fv1 *= vmult;
    std::cout << " and after: " << fv1 << std::endl;
    if (fv1 != morph::vec<float>({ 0.0f,0.0f,0.0f })) { --rtn; }

    morph::vec<float, 4> avec1 = { 1, 0, 0, 0 };
    morph::vec<float, 4> avec2 = { 0, 0, 1, 0 };
    std::cout << "angle (degrees) between " << avec1 << " and " << avec2 << " is "
              << (avec1.angle(avec2) * morph::mathconst<float>::rad2deg)
              << " or " << (avec2.angle(avec1) * morph::mathconst<float>::rad2deg) << std::endl;

    morph::vec<float, 3> testvec = {1, 0, 0};
    morph::vec<float, 3> othervec = {0.707, 0.707, 0};
    float tv_ov = testvec.angle(othervec, morph::vec<float>{0, 0, 1});
    std::cout << othervec << " is at angle " << tv_ov << " wrt " << testvec << " around the uz axis" << std::endl;
    if (tv_ov < 0.0f) { --rtn; }
    float ov_tv = othervec.angle(testvec, morph::vec<float>{0, 0, 1});
    std::cout << testvec << " is at angle " << ov_tv << " wrt " << othervec << " around the uz axis" << std::endl;
    if (ov_tv > 0.0f) { --rtn; }

    return rtn;
}
