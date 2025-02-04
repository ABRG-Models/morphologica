#include "morph/vvec.h"
#include "morph/vec.h"
#include "morph/mathconst.h"
#include <array>
#include <cstdint>
using morph::vvec;
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    vvec<float> v = {{1.0f,2.0f,3.0f}};
    // Test x(), y() etc members
    cout << "x: " << v.x() << endl;
    cout << "z: " << v.z() << endl;
#ifndef __WIN__ // may cause debug assertion, so lets avoid that
    cout << "w: " << v.w() << endl;
#endif
    // Test renormalize
    v.renormalize();
    cout << "After renormalize: " << v << endl;
    // Test checkunit
    cout << "is it unit? " << v.checkunit() << endl;
    // Test randomize
    v.randomize();
    cout << "After randomize: " << v << endl;
    // Check ints are ok, too
    vvec<int> vi(2);
    vi.randomize(0,100);
    cout << "After randomize of int vector: " << vi << endl;
    cout << "Length: " << vi.length() << endl;

    morph::vec<float, 2> vfl = {113, 124};
    cout << "Length of a float morph::vec: " << vfl.length() << endl;
    morph::vvec<float> vvfl = {113, 124};
    cout << "Length of a float morph::vvec: " << vvfl.length() << endl;

    morph::vec<int, 2> vil = {113, 124};
    cout << "Length of an int morph::vec: " << vil.length() << endl;
    morph::vvec<int> vvil = {113, 124};
    cout << "Length of an int morph::vvec: " << vvil.length() << endl;

    // Test assignment
    vvec<int> vi2 = vi;
    cout << "Copy of int vector: " << vi2 << endl;
    // Test comparison
    cout << "vi == vi2? " << (vi == vi2 ? "yes" : "no") << endl;
    // Test negate
    vvec<int> vi3(2);
    vi3 = -vi;
    vvec<int> vi33 = -vi;
    cout << "-ve Copy of int vector: " << vi3 << endl;
    // Test comparison
    cout << "vi == vi3? " << (vi == vi3 ? "yes" : "no") << endl;
    // Test cross product (3D only
    vvec<double> a = {1.0, 0.0, 0.0};
    vvec<double> b = {0.0, 1.0, 0.0};
    vvec<double> c = a.cross(b);
    cout << a << " cross " << b << "=" << c << endl;
    // Test dot product
    vvec<int> vv1 = {1,1};
    vvec<int> vv2 = {2,2};
    int dp = vv1.dot (vv2);
    cout << vv1 << "." << vv2 << " = " << dp << endl;

    // dot product of two different sized vectors
    vvec<int> vv2_3 = {2,2,2};
    try {
        int dpmm = vv1.dot (vv2_3);
        cout << vv1 << "." << vv2_3 << " = " << dpmm << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

#if 0 // No good here either, but may be able to fix by overloading operator=
    // Test init from array
    array<float, 3> arr = { 2,3,4 };
    std::vector<float> veccy(3);
    vvec<float> varr = arr; // Tried overloading operator= to no avail.
    cout << "vvec from array: " << varr << endl;
#endif

#if 0 // Haven't figured out assignment from a vector
    std::vector<float> veccy(3); veccy[0]=3.0f;
    vvec<float> varr = veccy;
#endif

    // Test scalar multiply
    vv2 *= 2UL;
    cout << "vv2 after *2:" << vv2 << endl;
    vvec<int> vv4 = vv1 * (int)98;
    cout << vv1 << " * 98:" << vv4 << endl;
    // Scalar division
    vvec<double> d = a/3.0;
    cout << "a/3.0:" << d << endl;
    // vvec addition
    vvec<double> e = a+b;
    cout << "a+b:" << e << endl;
    // vvec subtraction
    vvec<double> f = a-b;
    cout << "a-b:" << f << endl;
    // Test default template args
    vvec<double> vd_def;
    vd_def.randomize();
    cout << vd_def << endl;
    vvec<> v_def;
    v_def.randomize();
    cout << v_def << endl;

    // So you want to do the dot product of a 1000000 D vector? Easy
    vvec<float> big1(1000);
    vvec<float> big2(1000);
    big1.randomize(0,10);
    big2.randomize(0,10.0f);
    cout << "DP..." << endl;
    float bdp = big1.dot(big2);
    cout << "big1.big2=" << bdp << endl;

    // Test setFrom
    vvec<double> d1;
    array<double, 3> a1 = { 5,6,7 };
    d1.set_from (a1);
    cout << "d1 should be 5,6,7: " << d1 << endl;
    array<double, 4> a2 = { 5,6,8,8 };
    d1.set_from_onelonger(a2);
    std::cout << "d1.set_from_onelonger(a2) gives d1: " << d1 << std::endl;
    if (!(d1[0] == 5 && d1[1] == 6 && d1[2] == 8)) {
        std::cout << "fail this one\n"; --rtn;
    }

    vvec<double> d1cpy = d1;
    std::vector<float> a2longer = { 7, 8, 9, 9 };
    d1cpy.set_from_onelonger (a2longer);
    std::cout << "d1cpy.set_from_onelonger(a2longer) gives d1cpy: " << d1cpy << std::endl;
    if (d1cpy[2] != 9) { std::cout << "and fail this one\n"; --rtn; }

    vvec<int> v2longer = { 10, 100, 1000, 1000 };
    d1cpy.set_from_onelonger (v2longer);
    std::cout << "d1cpy.set_from_onelonger(v2longer) gives d1cpy: " << d1cpy << std::endl;
    if (d1cpy[2] != 1000) { std::cout << "and fail this one\n"; --rtn; }

    std::array<int, 4> aa2longer = { 100, 1000, 10000, 10000 };
    d1cpy.set_from_onelonger (aa2longer);
    std::cout << "d1cpy.set_from_onelonger(aa2longer) gives d1cpy: " << d1cpy << std::endl;
    if (d1cpy[2] != 10000) { std::cout << "and fail this one\n"; --rtn; }



    cout << "d1 should be 5,6,8: " << d1 << endl;
    d1.set_from (88.3);
    cout << "d1 should be 88.3 in all elements: " << d1 << endl;

    // Test hadamard operator* (elementwise multiplication)
    vvec<double> h1 = {1.0, 2.0, 3.0};
    vvec<double> h2 = {7.0, 6.0, 5.0};
    vvec<double> h3 = h1 * h2;
    cout << h1 << "(o)" << h2 << " = " << h3 << endl;

    h1 *= h2;
    cout << "After h1 *= h2, h1: " << h1 << endl;

    // Test operator *= with different types. Ok if lhs is same type as result.
    vvec<int> h4 = {2, 2, 2};
    //vvec<int> h5 = h2 * h4; // Not ok
    vvec<int> h6 = h4 * h2;
    vvec<double> h7 = h2 * h4;
    //vvec<double> h8 = h4 * h2; // Not ok
    cout << h2 << "(o)" << h4 << " = " << h6 << " or " << h7 << endl;

    // Operator* and operator*= with different length vectors
    vvec<double> dl1 = {2.0, 3.0, 4.0};
    vvec<double> dl2 = {2.0, 3.0};
    try {
        vvec<double> dlresult = dl1 * dl2;
        cout << dl1 << " * " << dl2 << " = " << dlresult << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

    vvec<double> dl1_ = {2.0, 3.0};
    vvec<double> dl2_ = {2.0, 3.0, 4.0};
    try {
        vvec<double> dlresult_ = dl1_ * dl2_;
        cout << dl1_ << " * " << dl2_ << " = " << dlresult_ << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

    try {
        dl1_ *= dl2_;
        cout << "{2, 3} *= {2, 3, 4} gives " << dl1_ << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

    try {
        dl2_ *= dl2;
        cout << "{2, 3, 4} *= {2, 3} gives " << dl2_ << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

    // Signum function
    vvec<float> sigtest = { -1.2, 0.001, 0.0f, 34.0f, -1808.8f };
    std::cout << "signum of " << sigtest << " is " << sigtest.signum() << std::endl;
    vvec<float> sigexpect = { -1.0f, 1.0f, 0.0f, 1.0f, -1.0f };
    if (sigtest.signum() != sigexpect) { --rtn; }
    std::cout << "signum of " << sigtest << ", computed in place, is ";
    sigtest.signum_inplace();
    std::cout << sigtest << std::endl;
    if (sigtest != sigexpect) { --rtn; }

    // Raising to a power
    vvec<float> powtest = { 1.1f, 2.3f, 4.7 };
    vvec<unsigned int> powrs = { 2, 3, 4 };
    std::cout << "Powers: " << powtest << " raised to powers " << powrs << " is " << powtest.pow(powrs) << std::endl;
    std::cout << "After, powtest is still " << powtest << " and after .pow_inplace() is ";
    powtest.pow_inplace(powrs);
    std::cout << powtest << std::endl;

    // Less than/gtr than operator
    vvec<double> lttest = { 0, -1, 2, 3.4, 3.8, 6.0 };
    std::cout << "Considering ALL elements of " << lttest << ":\n";
    std::cout << (lttest < 3.5 ? "  ALL are less than 3.5" : "  NOT ALL are less than 3.5") << std::endl;
    std::cout << (lttest < 6.2 ? "  ALL are less than 6.2" : "  NOT ALL are less than 6.2") << std::endl;
    std::cout << (lttest < 6.0 ? "  ALL are less than 6.0" : "  NOT ALL are less than 6.0") << std::endl;
    std::cout << (lttest > 3.5 ? "  ALL are greater than 3.5" : "  NOT ALL are greater than 3.5") << std::endl;
    std::cout << (lttest > 6.2 ? "  ALL are greater than 6.2" : "  NOT ALL are greater than 6.2") << std::endl;
    std::cout << (lttest > -1.1 ? "  ALL are greater than -1.1" : "  NOT ALL are greater than -1.1") << std::endl;
    vvec<double> ltthan = { 1, -2, 1, 4.4, 3.8, 5.8};
    std::cout << "Considering ALL elements of " << lttest
              << "\ncompared with                " << ltthan << ":\n";
    std::cout << " ALL less than? " << (lttest < ltthan ? "True" : "False") << std::endl;
    std::cout << " ALL gtr than? " << (lttest > ltthan ? "True" : "False") << std::endl;

    vvec<double> ltthanplus = ltthan + 1.0;
    std::cout << "ltthan + 1 > lthan? " << (ltthanplus > ltthan ? "True" : "False") << std::endl;
    std::cout << "ltthan + 1 < lthan? " << (ltthanplus < ltthan ? "True" : "False") << std::endl;

    std::cout << "ltthan + 1 == lthan? " << (ltthan == ltthanplus ? "True" : "False") << std::endl;
    vvec<double> ltthancopy = ltthan;
    std::cout << "Is a copy of a vvec == to the vvec? " << (ltthan == ltthancopy ? "True" : "False") << std::endl;

    std::cout << "twice " << ltthan << " = " << (2.0*ltthancopy) << std::endl;
    std::cout << "one over " << ltthan << " = " << (1.0/ltthancopy) << std::endl;
    std::cout << "one + " << ltthan << " = " << (1.0+ltthancopy) << std::endl;
    std::cout << "one - " << ltthan << " = " << (1.0-ltthancopy) << std::endl;

    vvec<double> ltt2 = { 1, 2 };
    vvec<double> ltt3 = { 1.1, 2.9 };
    std::cout << (ltt3 < ltt2 ? "Y" : "N") << std::endl;
    std::vector<double> ltt2v = { 1, 2 };
    std::vector<double> ltt3v = { 0.9, 1.9 };
    std::cout << (ltt3v < ltt2v ? "Y" : "N") << std::endl;

    std::vector<int> stdvec = { 1, 2, 3 };
    morph::vvec<int> fromstd;
    // You can't do fromstd = stdvec; instead, do this:
    fromstd.set_from (stdvec);

    vvec<double> lins;
    size_t nnn = 11;
    lins.linspace (0, 1, nnn);
    std::cout << nnn << " linearly spaced values from 0 to 1:\n" << lins << std::endl;
    vvec<float> linsi(12);
    linsi.linspace (23, 45);
    std::cout << linsi.size() << " linearly spaced float values from " << linsi[0]
              << " to " << linsi[linsi.size()-1] << ":\n" << linsi << std::endl;

    linsi.resize(34);
    linsi.linspace (-1, 1);
    std::cout << linsi.size() << " linearly spaced float values from " << linsi[0]
              << " to " << linsi[linsi.size()-1] << ":\n" << linsi << std::endl;

    linsi.linspace (1, -1);
    std::cout << linsi.size() << " linearly spaced float values from " << linsi[0]
              << " to " << linsi[linsi.size()-1] << ":\n" << linsi << std::endl;


#ifndef __WIN__ // VS doesn't like the dot product here. I can't figure out what's wrong
    // Test different vvec  types dotted:
    vvec<double> left = h1;
    vvec<int> right = { 2,2,3 };
    double dotprod = left.dot(right);
    cout << h1 << "." << right << " = " << dotprod << endl;
#endif

    vvec<float> maxlongest = {-1.1f, -7.0f, 3.0f, 6.0f };
    cout << "For vector " << maxlongest
         << ", max: " << maxlongest.max() << " (at index "<< maxlongest.argmax()
         << "), longest component: " << maxlongest.longest() << " (at index "
         << maxlongest.arglongest() << ")\n";
    cout << "For vector " << maxlongest
         << ", min: " << maxlongest.min() << " (at index "<< maxlongest.argmin()
         << "), shortest component: " << maxlongest.shortest() << " (at index "
         << maxlongest.argshortest() << ")\n";

    vvec<double> forshortest = { 2.9, 0, -1.1, 3.9 };
    cout << "For vector " << forshortest << std::endl;
    cout << "  Shortest: " << forshortest.shortest() << std::endl;
    cout << "  Shortest non-zero: " << forshortest.shortest_nonzero() << std::endl;
    if (forshortest.shortest_nonzero() != -1.1) { --rtn; }

    // Ensure it works if 0 comes first
    forshortest = { 0, 2.9, -1.1, 3.9 };
    cout << "For vector " << forshortest << std::endl;
    cout << "  Shortest: " << forshortest.shortest() << std::endl;
    cout << "  Shortest non-zero: " << forshortest.shortest_nonzero() << std::endl;
    if (forshortest.shortest_nonzero() != -1.1) { --rtn; }

    // Ensure it works if 0 comes last
    forshortest = { 2.9, -1.1, 3.9, 0 };
    cout << "For vector " << forshortest << std::endl;
    cout << "  Shortest: " << forshortest.shortest() << std::endl;
    cout << "  Shortest non-zero: " << forshortest.shortest_nonzero() << std::endl;
    if (forshortest.shortest_nonzero() != -1.1) { --rtn; }

    vvec<morph::vec<float, 2>> forshortestvec = { {0, 0}, {0, 0}, {1, 1}, {1, 2} };
    cout << "For vector " << forshortestvec  << std::endl;
    cout << "  Shortest: " << forshortestvec .shortest() << std::endl;
    cout << "  Shortest non-zero: " << forshortestvec .shortest_nonzero() << std::endl;
    if (forshortestvec .shortest_nonzero() != morph::vec<float, 2>{1, 1}) { --rtn; }

    forshortestvec = { {1, 1}, {0, 0}, {0, 0}, {1, 1}, {1, 2} };
    cout << "For vector " << forshortestvec  << std::endl;
    cout << "  Shortest: " << forshortestvec .shortest() << std::endl;
    cout << "  Shortest non-zero: " << forshortestvec .shortest_nonzero() << std::endl;
    if (forshortestvec .shortest_nonzero() != morph::vec<float, 2>{1, 1}) { --rtn; }

    vvec<float> cc = { 1.0f, 2.0f };
    float D = 2.0f;
    std::cout << "(-cc/D).exp()=" << (-cc/D).exp() << std::endl;
    std::cout << "(-cc)/D=" << ((-cc)/D) << std::endl;

    // Cast to std::vector
    std::vector<float>& rv = static_cast<std::vector<float>&>(cc);
    std::vector<float> rv2 = static_cast<std::vector<float>>(cc);
    std::cout << "cast a vvec " << cc << " to std::vector:\n";
    std::cout << "(" << rv[0] << "," << rv[1] << ")" << std::endl;
    for (auto rvi : rv2) { std::cout << rvi << std::endl; }

    // Convert precision
    cc = {1.234523452345f, 5.23452345345f};
    vvec<double> ddcc = cc.as_double();
    std::cout << "cc: " << cc << " cc.as_double(): " << ddcc << " and back to single " << ddcc.as_float() << std::endl;
    ddcc = {1.2345234755654907,5.2345232963562812};
    std::cout << "double prec: " << ddcc << " to single: " << ddcc.as_float() << "\n   and back: " << ddcc.as_float().as_double() << std::endl;

    // Rotate
    vvec<int>vvir = { 1, 2, 3, 4 };
    vvec<int> vvir1 = vvir;
    vvir1.rotate();
    std::cout << vvir << " rotate(): " << vvir1 << std::endl;

    vvec<int> vvir2(vvir);
    for (size_t n = 0; n < 6; ++n) {
        vvir2 = vvir;
        vvir2.rotate (n);
        std::cout << vvir << " rotate("<<n<<"): " << vvir2 << std::endl;
    }

    for (int n = -6; n < 7; ++n) {
        vvir2 = vvir;
        vvir2.rotate (n);
        std::cout << vvir << " rotate("<<n<<"): " << vvir2 << std::endl;
    }

    morph::vvec<int> vr = { 0, 1, 2, 3 };
    morph::vvec<int> vr2 = vr;

    morph::vvec<int> rot_size_t_correct = { 0, 1, 2, 3, 0, 1, 2 };
    for (size_t i = 0; i < 7; ++i) {
        vr2 = vr;
        vr2.rotate (i);
        std::cout << vr << " rotate("<<i<<") is " << vr2 << std::endl;
        if (vr2[0] != rot_size_t_correct[i]) { --rtn; }
    }

    morph::vvec<int> rot_int_correct = { 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2 };
    for (int i = -7; i < 7; ++i) {
        vr2 = vr;
        vr2.rotate (i);
        std::cout << vr << " rotate("<<i<<") is " << vr2 << std::endl;
        if (vr2[0] != rot_int_correct[i+7]) { --rtn; }
    }

    vvec<float> vfr(81, 0.0f);
    vfr.linspace (-morph::mathconst<float>::pi, morph::mathconst<float>::pi, 81);
    vfr.cos_inplace();
    std::cout << "PRE: " << vfr << std::endl;
    vfr.rotate (static_cast<int>(-1));
    if (vfr[0] != -1.0f || vfr[1] != -1.0f) { --rtn; }
    std::cout << "POST: " << vfr << std::endl;

    vvec<float> formax (3, 0.0f);
    formax.set_max();
    std::cout << "vvec<float>::set_max gives: " << formax << std::endl;
    formax.set_lowest();
    std::cout << "vvec<float>::set_lowest gives: " << formax << std::endl;

    // Concat two diff. sized vectors
    vvec<unsigned int> ua = { 3, 4, 5 };
    vvec<unsigned int> ub = { 30, 40, 50, 60 };
    // Expected result:
    vvec<unsigned int> uab_cmp = { 3, 4, 5, 30, 40, 50, 60 };
    std::cout << "Before concat(), ua is " << ua << std::endl;
    ua.concat (ub);
    std::cout << "After concat" << ub << ", ua is " << ua << std::endl;
    if (uab_cmp != ua) { --rtn; }

    // Concat onto empty vvec
    ua = {};
    uab_cmp = { 30, 40, 50, 60 };
    std::cout << "Before concat(), ua is " << ua << std::endl;
    ua.concat (ub);
    std::cout << "After concat" << ub << ", ua is " << ua << std::endl;
    if (uab_cmp != ua) { --rtn; }

    // Concat empty vvec
    ua = { 3, 4, 5 };
    ub = {};
    uab_cmp = { 3, 4, 5 };
    std::cout << "Before concat(), ua is " << ua << std::endl;
    ua.concat (ub);
    std::cout << "After concat" << ub << ", ua is " << ua << std::endl;
    if (uab_cmp != ua) { --rtn; }

    // Concat 2 empty vvecs
    ua = {};
    ub = {};
    uab_cmp = {};
    std::cout << "Before concat(), ua is " << ua << std::endl;
    ua.concat (ub);
    std::cout << "After concat" << ub << ", ua is " << ua << std::endl;
    if (uab_cmp != ua) { --rtn; }

    // Test shorten
    vvec<float> lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    vvec<float> sv = lv.shorten (5.0f);
    std::cout << "lv: " << lv << " lv.shorten(5.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 3.0f, 4.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (10.0f);
    std::cout << "lv: " << lv << " lv.shorten(10.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (12.0f);
    std::cout << "lv: " << lv << " lv.shorten(12.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.shorten (-5.0f); // shorten -ve lengthens
    std::cout << "lv: " << lv << " lv.shorten(-5.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 9.0f, 12.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-5.0f); // lengthen -ve shortens
    std::cout << "lv: " << lv << " lv.lengthen(-5.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 3.0f, 4.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-10.0f);
    std::cout << "lv: " << lv << " lv.lengthen(-10.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (-12.0f);
    std::cout << "lv: " << lv << " lv.lengthen(-12.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 0.0f, 0.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (5.0f);
    std::cout << "lv: " << lv << " lv.lengthen(5.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 9.0f, 12.0f })) { --rtn; }

    lv = { 6.0f, 8.0f }; // a 3,4,5 vector
    sv = lv.lengthen (15.0f);
    std::cout << "lv: " << lv << " lv.lengthen(15.0f) returns the vector: " << sv << std::endl;
    if (sv != vvec<float>({ 15.0f, 20.0f })) { --rtn; }

    // Test zeroing
    vvec<float> vzero = {1,2,3};
    vzero.zero();
    std::cout << "After zero, vzero = " << vzero << std::endl;
    if (vzero.sum() != 0.0f) { --rtn; }

    // What about a vvec of vecs?
    vvec<morph::vec<int, 2>> vvvec = { {1,2}, {3,4} };
    std::cout << "Before zero: " << vvvec << " with sum " << vvvec.sum().sum() << std::endl;

    vvvec.zero();
    if (vvvec.sum().sum() != 0) { --rtn; }
    std::cout << "After zero: " << vvvec << " with sum " << vvvec.sum().sum() << std::endl;

    // Sum of squares
    vvec<uint8_t> sos1 = {2, 3, 4, 5};
    std::cout << sos1.as_uint() << " uint8_t sum of squares: sos1.sos(): " << sos1.sos() << std::endl;
    std::cout << sos1.as_uint() << " uint8_t sum of squares: sos1.sos<unsigned int>(): " << sos1.sos<unsigned int>() << std::endl;

    std::cout << sos1.as_uint() << " uint8_t to power 3: sos1.pow<uint8_t>(4) = " << sos1.pow<uint8_t>(4).as_uint() << std::endl;
    std::cout << sos1.as_uint() << " uint8_t to power 3: sos1.pow<unsigned int>(4) = " << sos1.pow<unsigned int>(4) << std::endl;

    // Correctly fails to compile/errors (will fail to compile when morph moves to C++-20)
    try {
        std::cout << sos1.as_uint() << " uint8_t sum of squares: length_sq.sos<vec<>>(): " << sos1.length_sq<morph::vec<float, 2>>() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }

    vvec<morph::vec<int, 2>> sosv1 = {{1,2}, {3,2}, {2,4}};
    //std::cout << sosv1 << " sum of squares: " << sosv1.sos() << std::endl; // won't compile or will runtime error
    std::cout << sosv1 << " is a vector of vectors, so sosv1.length_sq<int>() returns a sum of squared lengths: " << sosv1.length_sq<int>() << std::endl;
    std::cout << sosv1 << " is a vector of vectors, so sosv1.sos(): " << sosv1.sos() << std::endl;

    morph::vvec<uint8_t> uv = {10, 10, 10};
    std::cout << uv.as_uint() << ".product() = " << static_cast<unsigned int>(uv.product()) << std::endl;
    std::cout << uv.as_uint() << ".product<unsigned int>() = " << uv.product<unsigned int>() << std::endl;

    morph::vvec<uint8_t> uv2 = {1, 2, 10, 3, 11, 23};
    std::cout << uv2.as_uint() << " mean: " << uv2.mean<float>() << std::endl;
    std::cout << uv2.as_uint() << " variance: " << uv2.variance<float>() << std::endl;

    morph::vvec<float> uv2f = {1, 2, 10, 3, 11, 23};
    std::cout << uv2f << " mean: " << uv2f.mean() << std::endl;
    std::cout << uv2f << " variance: " << uv2f.variance() << std::endl;

    std::cout << "At end, rtn=" << rtn << std::endl;
    return rtn;
}
