#include "morph/vVector.h"
#include <array>
using morph::vVector;
using std::cout;
using std::endl;
using std::array;

int main() {
    int rtn = 0;
    vVector<float> v = {{1.0f,2.0f,3.0f}};
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
    v.randomize();
    cout << "After randomize: " << v << endl;
    // Check ints are ok, too
    vVector<int> vi(2);
    vi.randomize(0,100);
    cout << "After randomize of int vector: " << vi << endl;
    cout << "Length: " << vi.length() << endl;
    // Test assignment
    vVector<int> vi2 = vi;
    cout << "Copy of int vector: " << vi2 << endl;
    // Test comparison
    cout << "vi == vi2? " << (vi == vi2 ? "yes" : "no") << endl;
    // Test negate
    vVector<int> vi3(2);
    vi3 = -vi;
    vVector<int> vi33 = -vi;
    cout << "-ve Copy of int vector: " << vi3 << endl;
    // Test comparison
    cout << "vi == vi3? " << (vi == vi3 ? "yes" : "no") << endl;
    // Test cross product (3D only
    vVector<double> a = {1,0,0};
    vVector<double> b = {0,1,0};
    vVector<double> c = a.cross(b);
    cout << a << " cross " << b << "=" << c << endl;
    // Test dot product
    vVector<int> vv1 = {1,1};
    vVector<int> vv2 = {2,2};
    int dp = vv1.dot (vv2);
    cout << vv1 << "." << vv2 << " = " << dp << endl;

    // dot product of two different sized vectors
    vVector<int> vv2_3 = {2,2,2};
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
    vVector<float> varr = arr; // Tried overloading operator= to no avail.
    cout << "vVector from array: " << varr << endl;
#endif

#if 0 // Haven't figured out assignment from a vector
    std::vector<float> veccy(3); veccy[0]=3.0f;
    vVector<float> varr = veccy;
#endif

    // Test scalar multiply
    vv2 *= 2UL;
    cout << "vv2 after *2:" << vv2 << endl;
    vVector<int> vv4 = vv1 * (int)98;
    cout << vv1 << " * 98:" << vv4 << endl;
    // Scalar division
    vVector<double> d = a/3.0;
    cout << "a/3.0:" << d << endl;
    // vVector addition
    vVector<double> e = a+b;
    cout << "a+b:" << e << endl;
    // vVector subtraction
    vVector<double> f = a-b;
    cout << "a-b:" << f << endl;
    // Test default template args
    vVector<double> vd_def;
    vd_def.randomize();
    cout << vd_def << endl;
    vVector<> v_def;
    v_def.randomize();
    cout << v_def << endl;

    // So you want to do the dot product of a 1000000 D vector? Easy
    vVector<float> big1(1000);
    vVector<float> big2(1000);
    big1.randomize(0,10);
    big2.randomize(0,10.0f);
    cout << "DP..." << endl;
    float bdp = big1.dot(big2);
    cout << "big1.big2=" << bdp << endl;

    // Test setFrom
    vVector<double> d1;
    array<double, 3> a1 = { 5,6,7 };
    d1.set_from (a1);
    cout << "d1 should be 5,6,7: " << d1 << endl;
    array<double, 4> a2 = { 5,6,8,8 };
    d1.set_from_onelonger(a2);
    cout << "d1 should be 5,6,8: " << d1 << endl;
    d1.set_from (88.3);
    cout << "d1 should be 88.3 in all elements: " << d1 << endl;

    // Test hadamard operator* (elementwise multiplication)
    vVector<double> h1 = {1.0, 2.0, 3.0};
    vVector<double> h2 = {7.0, 6.0, 5.0};
    vVector<double> h3 = h1 * h2;
    cout << h1 << "(o)" << h2 << " = " << h3 << endl;

    h1 *= h2;
    cout << "After h1 *= h2, h1: " << h1 << endl;

    // Test operator *= with different types. Ok if lhs is same type as result.
    vVector<int> h4 = {2, 2, 2};
    //vVector<int> h5 = h2 * h4; // Not ok
    vVector<int> h6 = h4 * h2;
    vVector<double> h7 = h2 * h4;
    //vVector<double> h8 = h4 * h2; // Not ok
    cout << h2 << "(o)" << h4 << " = " << h6 << " or " << h7 << endl;

    // Operator* and operator*= with different length vectors
    vVector<double> dl1 = {2.0, 3.0, 4.0};
    vVector<double> dl2 = {2.0, 3.0};
    try {
        vVector<double> dlresult = dl1 * dl2;
        cout << dl1 << " * " << dl2 << " = " << dlresult << endl;
    } catch (const std::exception& e) {
        cout << "Expected exception: ";
        cout << e.what() << endl;
    }

    vVector<double> dl1_ = {2.0, 3.0};
    vVector<double> dl2_ = {2.0, 3.0, 4.0};
    try {
        vVector<double> dlresult_ = dl1_ * dl2_;
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

    // Test different vVector types dotted:
    vVector<double> left = h1;
    vVector<int> right = { 2,2,3 };
    double dotprod = left.dot(right);
    cout << h1 << "." << right << " = " << dotprod << endl;

    vVector<float> maxlongest = {-1.1f, -7.0f, 3.0f, 6.0f };
    cout << "For vector " << maxlongest
         << ", max: " << maxlongest.max() << " (at index "<< maxlongest.argmax()
         << "), longest component: " << maxlongest.longest() << " (at index "
         << maxlongest.arglongest() << ")\n";
    cout << "For vector " << maxlongest
         << ", min: " << maxlongest.min() << " (at index "<< maxlongest.argmin()
         << "), shortest component: " << maxlongest.shortest() << " (at index "
         << maxlongest.argshortest() << ")\n";

    return rtn;
}
